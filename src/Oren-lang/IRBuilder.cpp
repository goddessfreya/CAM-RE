#include "IRBuilder.hpp"
#include "Parser.hpp"

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

OL::IRBuilder::IRBuilder(Parser* parser)
	: parser(parser),
	llvmIRBuilder(llvmContext)
{}

void OL::IRBuilder::BuildFile
(
	std::string filename,
	void* /*userData*/,
	CAM::WorkerPool* /*wp*/,
	size_t thread,
	CAM::Job* /*thisJob*/
)
{
	std::unique_ptr<llvm::Module> llvmModule;
	llvmModule = std::make_unique<llvm::Module>(filename, llvmContext);

	MakeStackFuncs(llvmModule.get());

	printf("%zu: Building %s\n", thread, filename.c_str());
	auto& AST = parser->GetASTForFile(filename);
	for (auto& node : AST)
	{
		auto ret = node->BuildCommand(llvmContext, llvmIRBuilder, llvmModule.get());

		if (ret == nullptr)
		{
			fprintf(stderr, "%zu: %s had unkown command\n", thread, filename.c_str());
		}
	}
}

llvm::Function* OL::IRBuilder::MakeFuncPrototype
(
	std::string name,
	llvm::Type* ret,
	std::vector<llvm::Type*>& params,
	std::vector<std::string>& paramNames,
	llvm::Module* llvmModule
)
{
	llvm::FunctionType* protType;
	if (params.empty())
	{
		protType = llvm::FunctionType::get(ret, false);
	}
	else
	{
		protType = llvm::FunctionType::get(ret, params, false);
	}

	llvm::Function* prot = llvm::Function::Create(protType, llvm::Function::ExternalLinkage, name, llvmModule);

	// Set names for all arguments.
	if (!params.empty())
	{
		unsigned i = 0;
		for (auto& arg : prot->args())
		{
			arg.setName(paramNames[i]);
			++i;
		}
	}

	return prot;
}

llvm::Function* OL::IRBuilder::MakeEmptyFunc
(
	std::string name,
	llvm::Type* ret,
	std::vector<llvm::Type*>& params,
	std::vector<std::string>& paramNames,
	llvm::Module* llvmModule
)
{
	assert(params.size() == paramNames.size());

	// Check for existing function declaration
	llvm::Function* func = llvmModule->getFunction(name);

	if (func == nullptr)
	{
		func = MakeFuncPrototype(name, ret, params, paramNames, llvmModule);
	}

	if (func == nullptr)
	{
		return nullptr;
	}

	if (!func->empty())
	{
		throw std::logic_error("Cannot redefine function.");
	}

	llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(llvmContext, "entry", func);
	llvmIRBuilder.SetInsertPoint(basicBlock);
	return func;
}

llvm::Value* OL::IRBuilder::GetParam(llvm::Function* func, std::string name)
{
	for (auto& arg : func->args())
	{
		if (arg.getName() == name)
		{
			return &arg;
		}
	}

	return nullptr;
}

void OL::IRBuilder::MakeStackFuncs(llvm::Module* llvmModule)
{
	auto stack = std::make_unique<llvm::GlobalVariable>
	(
		*llvmModule,
		llvm::ArrayType::get(llvm::IntegerType::get(llvmContext, 64), 8388608), // 8 mib
		false, // is constant
		llvm::GlobalValue::CommonLinkage,
		llvm::UndefValue::get(llvm::ArrayType::get(llvm::IntegerType::get(llvmContext, 64), 8388608)),
		"stack"
	);

	auto head = std::make_unique<llvm::GlobalVariable>
	(
		*llvmModule,
		llvm::IntegerType::get(llvmContext, 64),
		false, // is constant
		llvm::GlobalValue::CommonLinkage,
		llvm::UndefValue::get(llvm::IntegerType::get(llvmContext, 64)),
		"stack"
	);

	MakeStackPopFunc(llvmModule, stack.get(), head.get());
	MakeStackPushFunc(llvmModule, stack.get(), head.get());
}

void OL::IRBuilder::MakeStackPopFunc
(
	llvm::Module* llvmModule,
	llvm::GlobalVariable* /*stack*/,
	llvm::GlobalVariable* /*head*/
)
{
	std::vector<llvm::Type*> params{};
	std::vector<std::string> paramNames{};
	MakeEmptyFunc("pop", llvm::IntegerType::get(llvmContext, 64), params, paramNames, llvmModule);
}

void OL::IRBuilder::MakeStackPushFunc
(
	llvm::Module* llvmModule,
	llvm::GlobalVariable* stack,
	llvm::GlobalVariable* head
)
{
	std::vector<llvm::Type*> params{llvm::IntegerType::get(llvmContext, 64)};
	std::vector<std::string> paramNames{"val"};
	auto func = MakeEmptyFunc("push", llvm::Type::getVoidTy(llvmContext), params, paramNames, llvmModule);

	auto headv = llvmIRBuilder.CreateLoad(head, "head");
	auto addr = llvm::GetElementPtrInst::Create
	(
		llvm::ArrayType::get(llvm::IntegerType::get(llvmContext, 64), 8388608), // 8 mib
		stack,
		{
			llvm::ConstantInt::get(llvm::IntegerType::get(llvmContext, 64), 0),
			headv
		},
		"addr"
	);
	llvmIRBuilder.CreateStore(GetParam(func, "val"), addr);
	auto newheadv = llvmIRBuilder.CreateAdd
	(
		headv,
		llvm::ConstantInt::get(llvm::IntegerType::get(llvmContext, 64), 1),
		"newhead"
	);
	llvmIRBuilder.CreateStore(newheadv, head);
	llvmIRBuilder.CreateRetVoid();

	llvm::verifyFunction(*func);

	func->print(llvm::errs());
    fprintf(stderr, "\n");
}
