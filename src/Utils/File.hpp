#ifndef CAM_UTILS_FILE_HPP
#define CAM_UTILS_FILE_HPP

#include <cstdio>
#include <cstdint>
#include <string>
#include <stdexcept>

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

namespace CAM
{
namespace Utils
{
class File
{
	public:
	File(std::string file, std::string mode);
	~File();

	File(const File&) = delete;
	File(File&&) = default;
	File& operator=(const File&)& = delete;
	File& operator=(File&&)& = default;

	std::string GetContents();

	private:
	FILE* file;
};
}
}

#endif
