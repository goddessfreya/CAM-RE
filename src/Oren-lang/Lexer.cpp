#include "Lexer.hpp"
#include "Parser.hpp"

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

OL::Lexer::Lexer(Parser* parser) : parser(parser) {}

void OL::Lexer::Start
(
	void* userData,
	CAM::WorkerPool* wp,
	size_t thread,
	CAM::Job* thisJob
)
{
	printf("%zu: Lexer start.\n", thread);
	auto ud = static_cast<Lexer*>(userData);

	std::string filename;
	while ((filename = ud->GetFile()) != "")
	{
		using namespace std::placeholders;
		auto j = wp->GetJob
		(
			std::bind(&Lexer::LexFile, filename, _1, _2, _3, _4),
			userData,
			thisJob->NumberOfDepsOnMe()
		);

		for (auto& deps : thisJob->GetDepsOnMe())
		{
			j->DependsOnMe(deps);
		}

		wp->SubmitJob(std::move(j));
	}
}

void OL::Lexer::LexFile
(
	std::string filename,
	void* userData,
	CAM::WorkerPool* wp,
	size_t thread,
	CAM::Job* thisJob
)
{
	printf("%zu: Lexing %s\n", thread, filename.c_str());
	auto ud = static_cast<Lexer*>(userData);
	std::vector<Token> tokens;

	auto filecontents = CAM::Utils::File(filename, "r").GetContents();
	std::replace(std::begin(filecontents), std::end(filecontents), '\n', ' ');
	std::replace(std::begin(filecontents), std::end(filecontents), '\t', ' ');

	std::string delm = " ";

	std::string remainingStr = filecontents;
	auto end = remainingStr.find(delm);
	Token thisToken;
	thisToken.val = 0;
	while (end != std::string::npos)
	{
		auto str = remainingStr.substr(0, end);
		std::transform(std::begin(str), std::end(str), std::begin(str), ::tolower);

		if (str == "wtf")
		{
			tokens.push_back(thisToken);
			thisToken.val = 0;
		}
		else if (str == "why")
		{
			thisToken.val *= 2;
			++thisToken.val;
		}
		else if (str == "omg")
		{
			thisToken.val *= 2;
		}

		remainingStr = remainingStr.substr(end + 1);
		end = remainingStr.find(delm);
	}

	if (thisToken.val != 0)
	{
		tokens.push_back(thisToken);
		thisToken.val = 0;
		tokens.push_back(thisToken);
	}

	if (tokens.empty() || tokens.back().val != 0)
	{
		thisToken.val = 0;
		tokens.push_back(thisToken);
	}

	{
		std::unique_lock<std::mutex> lock(ud->tokensForFilesMutex);
		ud->tokensForFiles[filename] = std::move(tokens);
	}

	using namespace std::placeholders;
	auto j = wp->GetJob
	(
		std::bind(&Parser::ParseFile, filename, _1, _2, _3, _4),
		ud->parser,
		thisJob->NumberOfDepsOnMe()
	);

	for (auto& deps : thisJob->GetDepsOnMe())
	{
		j->DependsOnMe(deps);
	}

	wp->SubmitJob(std::move(j));
}

std::string OL::Lexer::GetFile()
{
	std::unique_lock<std::mutex> lock(filesMutex);
	if (files.empty())
	{
		return "";
	}

	auto ret = files.back();
	files.pop_back();
	return ret;
}

void OL::Lexer::SubmitFile(std::string file)
{
	std::unique_lock<std::mutex> lock(filesMutex);
	files.push_back(file);
}

const std::vector<OL::Token>& OL::Lexer::GetTokensForFile(std::string filename)
{
	std::unique_lock<std::mutex> lock(tokensForFilesMutex);
	return tokensForFiles[filename];
}
