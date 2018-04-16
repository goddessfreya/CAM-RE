/*
 * Copyright (C) 2018 Hal Gentz
 *
 * This file is part of CAM-RE.
 *
 * CAM-RE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Bash is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * CAM-RE. If not, see <http://www.gnu.org/licenses/>.
 */

#include "File.hpp"

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

	char buf[block] = {};
	std::string ret = "";

    while (fread(buf, 1, sizeof(buf), file) > 0)
	{
		ret += std::string(buf);
    }

	rewind(file);

	return ret;
}
