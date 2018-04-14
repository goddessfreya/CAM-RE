#include "Parser.hpp"
#include "Lexer.hpp"
#include "IRBuilder.hpp"

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

OL::Parser::Parser(Lexer* lexer, IRBuilder* irBuilder) : lexer(lexer), irBuilder(irBuilder) {}

void OL::Parser::ParseFile
(
	std::string filename,
	void* /*userData*/,
	CAM::WorkerPool* wp,
	size_t thread,
	CAM::Job* thisJob
)
{
	printf("%zu: Parsing %s\n", thread, filename.c_str());
	auto tokens = lexer->GetTokensForFile(filename);
	std::vector<std::unique_ptr<ASTNode>> AST;
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		if (tokens[i].val == PushCMD::OpCode)
		{
			printf("%zu: %lu Push %lu\n", thread, tokens[i].val, tokens[i + 1].val);
			++i;
			AST.push_back(std::make_unique<PushCMD>(tokens[i].val));
		}
		else if (tokens[i].val == PopCMD::OpCode)
		{
			printf("%zu: %lu Pop\n", thread, tokens[i].val);
			AST.push_back(std::make_unique<PopCMD>());
		}
		else if (tokens[i].val == NANDCMD::OpCode)
		{
			printf("%zu: %lu Nand\n", thread, tokens[i].val);
			AST.push_back(std::make_unique<NANDCMD>());
		}
		else if (tokens[i].val == DupeCMD::OpCode)
		{
			printf("%zu: %lu Dupe %lu\n", thread, tokens[i].val, tokens[i + 1].val);
			++i;
			AST.push_back(std::make_unique<DupeCMD>(tokens[i].val));
		}
		else if (tokens[i].val == OutCMD::OpCode)
		{
			printf("%zu: %lu Out\n", thread, tokens[i].val);
			AST.push_back(std::make_unique<OutCMD>());
		}
		else if (tokens[i].val == InCMD::OpCode)
		{
			printf("%zu: %lu In\n", thread, tokens[i].val);
			AST.push_back(std::make_unique<InCMD>());
		}
		else if (tokens[i].val == JumpCMD::OpCode)
		{
			printf("%zu: %lu Jump\n", thread, tokens[i].val);
			AST.push_back(std::make_unique<JumpCMD>());
		}
		else
		{
			printf("%zu: %lu Halt\n", thread, tokens[i].val);
			AST.push_back(std::make_unique<HaltCMD>());
		}
	}

	{
		std::unique_lock<std::mutex> lock(ASTPerFileMutex);
		ASTPerFile[filename] = std::move(AST);
	}

	using namespace std::placeholders;
	auto j = wp->GetJob
	(
		std::bind(&IRBuilder::BuildFile, irBuilder, filename, _1, _2, _3, _4),
		nullptr,
		thisJob->NumberOfDepsOnMe()
	);

	for (auto& deps : thisJob->GetDepsOnMe())
	{
		j->DependsOnMe(deps);
	}

	wp->SubmitJob(std::move(j));
}

const std::vector<std::unique_ptr<OL::ASTNode>>& OL::Parser::GetASTForFile(std::string filename)
{
	std::unique_lock<std::mutex> lock(ASTPerFileMutex);
	return ASTPerFile[filename];
}
