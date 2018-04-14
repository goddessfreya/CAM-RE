#ifndef CAM_AST_HPP
#define CAM_AST_HPP

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

#include "../Utils/Unused.hpp"

#include <cstdint>
#include <cstddef>

#define LLVM_ENABLE_THREADS

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"

namespace OL
{
class ASTNode
{
	public:
	inline virtual ~ASTNode() = 0;
	virtual llvm::Value* BuildCommand
	(
		llvm::LLVMContext& llvmContext,
		llvm::IRBuilder<>& llvmIRBuilder,
		llvm::Module* llvmModule
	) = 0;
};
inline ASTNode::~ASTNode() {}

class PushCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b001;

	PushCMD(uint64_t pushVal) : pushVal(pushVal) {}

	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& llvmContext,
		llvm::IRBuilder<>& llvmIRBuilder,
		llvm::Module* llvmModule
	) override
	{
		llvm::Function* callee = llvmModule->getFunction("push");
		if (!callee)
		{
			throw std::runtime_error("Push function not found");
		}
		std::vector<llvm::Value*> argsv{llvm::ConstantInt::get(llvm::IntegerType::get(llvmContext, 64), pushVal)};
		return llvmIRBuilder.CreateCall(callee, argsv, "calltmp");
	}

	private:
	uint64_t UNUSED(pushVal);
};

class PopCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b010;
	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& /*llvmContext*/,
		llvm::IRBuilder<>& llvmIRBuilder,
		llvm::Module* llvmModule
	) override
	{
		llvm::Function* callee = llvmModule->getFunction("pop");
		if (!callee)
		{
			throw std::runtime_error("Pop function not found");
		}
		std::vector<llvm::Value*> argsv{};
		return llvmIRBuilder.CreateCall(callee, argsv, "calltmp");
	}
};

class NANDCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b011;
	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& /*llvmContext*/,
		llvm::IRBuilder<>& /*llvmIRBuilder*/,
		llvm::Module* /*llvmModule*/
	) override {return nullptr;}
};

class DupeCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b100;

	DupeCMD(uint64_t dupeNum) : dupeNum(dupeNum) {}

	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& /*llvmContext*/,
		llvm::IRBuilder<>& /*llvmIRBuilder*/,
		llvm::Module* /*llvmModule*/
	) override {return nullptr;}

	private:
	uint64_t UNUSED(dupeNum);
};

class OutCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b101;
	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& /*llvmContext*/,
		llvm::IRBuilder<>& /*llvmIRBuilder*/,
		llvm::Module* /*llvmModule*/
	) override {return nullptr;}
};

class InCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b110;
	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& /*llvmContext*/,
		llvm::IRBuilder<>& /*llvmIRBuilder*/,
		llvm::Module* /*llvmModule*/
	) override {return nullptr;}
};

class JumpCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b111;
	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& /*llvmContext*/,
		llvm::IRBuilder<>& /*llvmIRBuilder*/,
		llvm::Module* /*llvmModule*/
	) override {return nullptr;}
};

class HaltCMD : public ASTNode
{
	public:
	llvm::Value* BuildCommand
	(
		llvm::LLVMContext& /*llvmContext*/,
		llvm::IRBuilder<>& /*llvmIRBuilder*/,
		llvm::Module* /*llvmModule*/
	) override {return nullptr;}
};
}

#endif
