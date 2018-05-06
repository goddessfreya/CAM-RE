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
#include "VKImageView.hpp"
#include "VKImage.hpp"

CAM::Renderer::VKImageView::VKImageView
(
	Jobs::WorkerPool* wp,
	Jobs::Job* /*thisJob*/,
	Renderer* parent,
	VkImageViewCreateInfo& createInfo,
	VKImage* image
) : parent(parent),
	wp(wp),
	device(parent->GetVKDevice()),
	image(image)
{
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.image = (*image)();

	VKFNCHECKRETURN(device->deviceVKFN->vkCreateImageView
	(
		(*device)(),
		&createInfo,
		nullptr,
		&vkImageView
	));
}

CAM::Renderer::VKImageView::~VKImageView()
{
	device->deviceVKFN->vkDestroyImageView((*device)(), vkImageView, nullptr);
}

