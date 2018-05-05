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
#include "VKSemaphore.hpp"
#include "VKDevice.hpp"

CAM::Renderer::VKSemaphore::VKSemaphore(Jobs::WorkerPool* wp, Jobs::Job* /*thisJob*/, Renderer* parent)
	: wp(wp), parent(parent)
{
	auto vkDevice = this->parent->GetVKDevice();
	VkSemaphoreCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

	VKFNCHECKRETURN(vkDevice->deviceVKFN->vkCreateSemaphore((*vkDevice)(), &createInfo, nullptr, &vkSemaphore));
}

CAM::Renderer::VKSemaphore::~VKSemaphore()
{
	auto vkDevice = this->parent->GetVKDevice();
	vkDevice->deviceVKFN->vkDestroySemaphore((*vkDevice)(), vkSemaphore, nullptr);
}
