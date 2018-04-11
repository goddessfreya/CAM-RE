#include "File.hpp"

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

CAM::Utils::File::File(std::string filename, std::string mode)
{
	file = fopen(filename.c_str(), mode.c_str());

	if (file == NULL)
	{
		throw std::runtime_error("Failed to open file.");
	}
}

CAM::Utils::File::~File()
{
	if (file != NULL)
	{
		fclose(file);
	}
}

std::string CAM::Utils::File::GetContents()
{
	const size_t block = 2048;

	char buf[block];
	std::string ret = "";

    while (fread(buf, 1, sizeof(buf), file) > 0)
	{
		ret += std::string(buf);
    }

	rewind(file);

	return ret;
}
