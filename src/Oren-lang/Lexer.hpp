#ifndef CAM_LEXER_HPP
#define CAM_LEXER_HPP

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
class Lexer
{
	public:
	static void Start(void* userData, CAM::WorkerPool* wp, size_t thread, CAM::Job* thisJob);

	std::string GetFile();
	void SubmitFile(std::string file);

	static void LexFile(std::string filename, void* userData, CAM::WorkerPool* wp, size_t thread, CAM::Job* thisJob);

	private:
	std::mutex tokensForFilesMutex;
	std::mutex filesMutex;
	std::vector<std::string> files;
	std::map<std::string, std::vector<Token>> tokensForFiles;
};
}

#endif
