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

#include "Renderer.hpp"
#include "VKFence.hpp"
#include "VKDevice.hpp"

CAM::Renderer::VKFence::VKFence(Jobs::WorkerPool* wp, Jobs::Job* /*thisJob*/, Renderer* parent)
	: wp(wp), parent(parent), device(parent->GetVKDevice())
{
	static int i = 0;
	thisI = ++i;
	auto vkDevice = this->parent->GetVKDevice();
	VkFenceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

	VKFNCHECKRETURN(vkDevice->deviceVKFN->vkCreateFence((*vkDevice)(), &createInfo, nullptr, &vkFence));
}

CAM::Renderer::VKFence::~VKFence()
{
	device->deviceVKFN->vkDestroyFence((*device)(), vkFence, nullptr);
}

bool CAM::Renderer::VKFence::IsReady()
{
	std::unique_lock<std::mutex> lock(vkFenceMutex);
	return VKFNCHECKRETURN(device->deviceVKFN->vkGetFenceStatus((*device)(), vkFence)) == VK_SUCCESS;
}

void CAM::Renderer::VKFence::Reset()
{
	std::unique_lock<std::mutex> lock(vkFenceMutex);
	VKFNCHECKRETURN(device->deviceVKFN->vkResetFences((*device)(), 1, &vkFence));
}

bool CAM::Renderer::VKFence::WaitFor(uint64_t timeout)
{
	std::unique_lock<std::mutex> lock(vkFenceMutex);

	return VKFNCHECKRETURN(device->deviceVKFN->vkWaitForFences
	(
		(*device)(),
		1,
		&vkFence,
		VK_FALSE,
		timeout
	)) == VK_SUCCESS;
}
