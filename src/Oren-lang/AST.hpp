#ifndef CAM_AST_HPP
#define CAM_AST_HPP

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

#include "../Utils/Unused.hpp"
#include "IRBuilder.hpp"

#include <cstdint>
#include <cstddef>

namespace OL
{
class ASTNode
{
	public:
	inline virtual ~ASTNode() = 0;
	virtual llvm::Value* BuildCommand(IRBuilderFileData* fileData) = 0;
};
inline ASTNode::~ASTNode() {}

class PushCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b001;

	PushCMD(uint64_t pushVal) : pushVal(pushVal) {}

	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;

	private:
	uint64_t UNUSED(pushVal);
};

class PopCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b010;
	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;
};

class NANDCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b011;
	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;
};

class DupeCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b100;

	DupeCMD(uint64_t dupeNum) : dupeNum(dupeNum) {}

	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;

	private:
	uint64_t UNUSED(dupeNum);
};

class OutCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b101;
	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;
};

class InCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b110;
	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;
};

class JumpCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b111;
	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;
};

class HaltCMD : public ASTNode
{
	public:
	llvm::Value* BuildCommand(IRBuilderFileData* fileData) override;
};
}

#endif