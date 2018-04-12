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

namespace OL
{
class ASTNode
{
	public:
	inline virtual ~ASTNode() = 0;
};
inline ASTNode::~ASTNode() {}

class PushCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b001;

	PushCMD(uint64_t pushVal) : pushVal(pushVal) {}

	private:
	UNUSED uint64_t pushVal;
};

class PopCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b010;
};

class NANDCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b011;
};

class DupeCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b100;

	DupeCMD(uint64_t dupeNum) : dupeNum(dupeNum) {}

	private:
	UNUSED uint64_t dupeNum;
};

class OutCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b101;
};

class InCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b110;
};

class JumpCMD : public ASTNode
{
	public:
	static const size_t OpCode = 0b111;
};

class HaltCMD : public ASTNode
{
	public:
};
}

#endif
