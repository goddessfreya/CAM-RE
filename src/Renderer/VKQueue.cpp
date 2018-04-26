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

#include "VKDevice.hpp"
#include "Renderer.hpp"
#include "VKQueue.hpp"

CAM::Renderer::VKQueue::VKQueue(Jobs::WorkerPool* wp, VKDevice* parent)
	: wp(wp),
	parent(parent)
{

}

CAM::Renderer::VKQueue::~VKQueue()
{

}
