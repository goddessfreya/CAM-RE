#ifndef CAM_PARSER_HPP
#define CAM_PARSER_HPP

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

#include "AST.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "../WorkerPool.hpp"
#include "../Job.hpp"

#include "../Utils/Unused.hpp"

namespace OL
{
class Lexer;

class Parser
{
	public:
	Parser(Lexer* lexer);

	static void ParseFile
	(
		std::string filename,
		void* userData,
		CAM::WorkerPool* wp,
		size_t thread,
		CAM::Job* thisJob
	);

	private:
	Lexer* lexer;
	std::vector<std::unique_ptr<ASTNode>> AST;
};

}

#endif
