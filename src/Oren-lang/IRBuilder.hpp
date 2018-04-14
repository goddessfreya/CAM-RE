#ifndef CAM_IRBUILDER_HPP
#define CAM_IRBUILDER_HPP

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

#include "../WorkerPool.hpp"
#include "../Job.hpp"

#include "../Utils/Unused.hpp"

#include "AST.hpp"

namespace OL
{
class Parser;

class IRBuilder
{
	public:
	IRBuilder(Parser* parser);

	void BuildFile
	(
		std::string filename,
		void* userData,
		CAM::WorkerPool* wp,
		size_t thread,
		CAM::Job* thisJob
	);

	private:
	llvm::Function* MakeFuncPrototype
	(
		std::string name,
		llvm::Type* ret,
		std::vector<llvm::Type*>& params,
		std::vector<std::string>& paramNames,
		llvm::Module* llvmModule
	);
	llvm::Function* MakeEmptyFunc
	(
		std::string name,
		llvm::Type* ret,
		std::vector<llvm::Type*>& params,
		std::vector<std::string>& paramNames,
		llvm::Module* llvmModule
	);

	llvm::Value* GetParam(llvm::Function* func, std::string name);

	void MakeStackFuncs(llvm::Module* llvmModule);
	void MakeStackPopFunc
	(
		llvm::Module* llvmModule,
		llvm::GlobalVariable* stack,
		llvm::GlobalVariable* head
	);
	void MakeStackPushFunc
	(
		llvm::Module* llvmModule,
		llvm::GlobalVariable* stack,
		llvm::GlobalVariable* head
	);

	Parser* parser;

	llvm::LLVMContext llvmContext;
	llvm::IRBuilder<> llvmIRBuilder;
};
}

#endif
