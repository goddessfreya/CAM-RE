#include "IRBuilder.hpp"
#include "Parser.hpp"

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

OL::IRBuilder::IRBuilder(Parser* parser) : parser(parser) {}

void OL::IRBuilder::BuildFile
(
	std::string filename,
	void* /*userData*/,
	CAM::WorkerPool* /*wp*/,
	size_t thread,
	CAM::Job* /*thisJob*/
)
{
	printf("%zu: Building %s\n", thread, filename.c_str());

	IRBuilderFileData* fileData;
	{
		auto fn = filename;
		std::unique_lock<std::mutex> lock(fileDatasMutex);
		fileDatas.insert({fn, std::make_unique<IRBuilderFileData>(filename)});
		fileData = fileDatas.at(filename).get();
	}

	MakeStackFuncs(fileData);

	std::vector<llvm::Type*> params{};
	std::vector<std::string> paramNames{};
	auto func = MakeEmptyFunc
	(
		"main_" + filename,
		llvm::IntegerType::get(fileData->llvmContext, 64),
		params,
		paramNames,
		false,
		fileData
	);

	auto& AST = parser->GetASTForFile(filename);
	for (auto& node : AST)
	{
		auto ret = node->BuildCommand(fileData);

		if (ret == nullptr)
		{
			fprintf(stderr, "%zu: %s had unkown command\n", thread, filename.c_str());
		}
	}

	llvm::verifyFunction(*func);

	std::string str;
	llvm::raw_string_ostream stream(str);
	func->print(stream);
    fprintf(stderr, "%s\n", stream.str().c_str());
}

llvm::Function* OL::IRBuilder::MakeFuncPrototype
(
	std::string name,
	llvm::Type* ret,
	std::vector<llvm::Type*>& params,
	std::vector<std::string>& paramNames,
	bool isVarg,
	IRBuilderFileData* fileData
)
{
	llvm::FunctionType* protType;
	protType = llvm::FunctionType::get(ret, params, isVarg);

	llvm::Function* prot = llvm::Function::Create
	(
		protType,
		llvm::Function::ExternalLinkage,
		name,
		fileData->llvmModule.get()
	);

	// Set names for all arguments.
	if (!params.empty())
	{
		unsigned i = 0;
		for (auto& arg : prot->args())
		{
			if (paramNames.size() == i)
			{
				return prot;
			}
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
	bool isVarg,
	IRBuilderFileData* fileData
)
{
	// Check for existing function declaration
	llvm::Function* func = fileData->llvmModule->getFunction(name);

	if (func == nullptr)
	{
		func = MakeFuncPrototype(name, ret, params, paramNames, isVarg, fileData);
	}

	if (func == nullptr)
	{
		return nullptr;
	}

	if (!func->empty())
	{
		throw std::logic_error("Cannot redefine function.");
	}

	llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create
	(
		fileData->llvmContext,
		"entry",
		func
	);
	fileData->llvmIRBuilder.SetInsertPoint(basicBlock);
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

void OL::IRBuilder::MakeStackFuncs(IRBuilderFileData* fileData)
{
	// We don't own these two, lack of unique_ptr is intensional
	fileData->stack = new llvm::GlobalVariable
	(
		*fileData->llvmModule.get(),
		llvm::ArrayType::get(llvm::IntegerType::get(fileData->llvmContext, 64), 8388608), // 8 mib
		false, // is constant
		llvm::GlobalValue::PrivateLinkage,
		llvm::UndefValue::get(llvm::ArrayType::get(llvm::IntegerType::get(fileData->llvmContext, 64), 8388608)),
		"stack"
	);

	fileData->head = new llvm::GlobalVariable
	(
		*fileData->llvmModule.get(),
		llvm::IntegerType::get(fileData->llvmContext, 64),
		false, // is constant
		llvm::GlobalValue::PrivateLinkage,
		llvm::UndefValue::get(llvm::IntegerType::get(fileData->llvmContext, 64)),
		"head"
	);

	MakeStackPopFunc(fileData);
	MakeStackPushFunc(fileData);
}

void OL::IRBuilder::MakeStackPopFunc
(
	IRBuilderFileData* fileData
)
{
	std::vector<llvm::Type*> params{};
	std::vector<std::string> paramNames{};
	auto func = MakeEmptyFunc
	(
		"pop",
		llvm::IntegerType::get(fileData->llvmContext, 64),
		params,
		paramNames,
		false,
		fileData
	);

	auto headv = fileData->llvmIRBuilder.CreateLoad(fileData->head, "head");
	auto newheadv = fileData->llvmIRBuilder.CreateSub
	(
		headv,
		llvm::ConstantInt::get(headv->getType(), 1),
		"newhead"
	);
	fileData->llvmIRBuilder.CreateStore(newheadv, fileData->head);
	auto addr = fileData->llvmIRBuilder.CreateGEP
	(
		llvm::ArrayType::get(llvm::IntegerType::get(fileData->llvmContext, 64), 8388608), // 8 mib,
		fileData->stack,
		{
			llvm::ConstantInt::get(headv->getType(), 0),
			newheadv
		},
		"addr"
	);
	auto retval = fileData->llvmIRBuilder.CreateLoad(addr, "retval");
	fileData->llvmIRBuilder.CreateRet(retval);

	llvm::verifyFunction(*func);

	std::string str;
	llvm::raw_string_ostream stream(str);
	func->print(stream);
    fprintf(stderr, "%s\n", stream.str().c_str());
}

void OL::IRBuilder::MakeStackPushFunc
(
	IRBuilderFileData* fileData
)
{
	std::vector<llvm::Type*> params{llvm::IntegerType::get(fileData->llvmContext, 64)};
	std::vector<std::string> paramNames{"val"};
	auto func = MakeEmptyFunc
	(
		"push",
		llvm::Type::getVoidTy(fileData->llvmContext),
		params,
		paramNames,
		false,
		fileData
	);

	auto headv = fileData->llvmIRBuilder.CreateLoad(fileData->head, "head");
	auto addr = fileData->llvmIRBuilder.CreateGEP
	(
		llvm::ArrayType::get(llvm::IntegerType::get(fileData->llvmContext, 64), 8388608), // 8 mib,
		fileData->stack,
		{
			llvm::ConstantInt::get(headv->getType(), 0),
			headv
		},
		"addr"
	);
	fileData->llvmIRBuilder.CreateStore(GetParam(func, "val"), addr);
	auto newheadv = fileData->llvmIRBuilder.CreateAdd
	(
		headv,
		llvm::ConstantInt::get(llvm::IntegerType::get(fileData->llvmContext, 64), 1),
		"newhead"
	);
	fileData->llvmIRBuilder.CreateStore(newheadv, fileData->head);
	fileData->llvmIRBuilder.CreateRetVoid();

	llvm::verifyFunction(*func);

	std::string str;
	llvm::raw_string_ostream stream(str);
	func->print(stream);
    fprintf(stderr, "%s\n", stream.str().c_str());
}
