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

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <utility>
#include <string>
#include <algorithm>

namespace OL
{
class Parser;

struct IRBuilderFileData
{
	IRBuilderFileData(std::string filename) : llvmIRBuilder(llvmContext)
	{
		llvmModule = std::make_unique<llvm::Module>(filename, llvmContext);
	}

	llvm::LLVMContext llvmContext;
	llvm::IRBuilder<> llvmIRBuilder;

	// We don't own these two, lack of unique_ptr is intensional
	llvm::GlobalVariable* stack;
	llvm::GlobalVariable* head;

	std::unique_ptr<llvm::Module> llvmModule;

	bool firstPrint = true;
	llvm::Constant* outFormatContents;
	llvm::GlobalVariable* outFormatString;
	llvm::Function* printfFunc;
};

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

	static llvm::Function* MakeFuncPrototype
	(
		std::string name,
		llvm::Type* ret,
		std::vector<llvm::Type*>& params,
		std::vector<std::string>& paramNames,
		bool isVarg,
		IRBuilderFileData* fileData
	);

	static llvm::Function* MakeEmptyFunc
	(
		std::string name,
		llvm::Type* ret,
		std::vector<llvm::Type*>& params,
		std::vector<std::string>& paramNames,
		bool isVarg,
		IRBuilderFileData* fileData
	);

	private:
	llvm::Value* GetParam(llvm::Function* func, std::string name);

	void MakeStackFuncs(IRBuilderFileData* fileData);
	void MakeStackPopFunc(IRBuilderFileData* fileData);
	void MakeStackPushFunc(IRBuilderFileData* fileData);

	Parser* parser;

	std::mutex fileDatasMutex;
	std::map<std::string, std::unique_ptr<IRBuilderFileData>> fileDatas;
};
}

#endif
