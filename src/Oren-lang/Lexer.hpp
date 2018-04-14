#ifndef CAM_LEXER_HPP
#define CAM_LEXER_HPP

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

#include "../WorkerPool.hpp"
#include "../Job.hpp"

#include "Token.hpp"
#include "../Utils/File.hpp"

#include <map>
#include <algorithm>
#include <string>

/*
 * This file is part of Oren-lang and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

namespace OL
{
class Parser;

class Lexer
{
	public:
	Lexer(Parser* parser);

	void Start
	(
		void* userData,
		CAM::WorkerPool* wp,
		size_t thread,
		CAM::Job* thisJob
	);

	std::string GetFile();
	void SubmitFile(std::string file);

	void LexFile
	(
		std::string filename,
		void* userData,
		CAM::WorkerPool* wp,
		size_t thread,
		CAM::Job* thisJob
	);

	const std::vector<Token>& GetTokensForFile(std::string filename);

	private:
	std::mutex tokensForFilesMutex;
	std::mutex filesMutex;
	std::vector<std::string> files;
	std::map<std::string, std::vector<Token>> tokensForFiles;
	Parser* parser;
};
}

#endif
