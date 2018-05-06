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
#include "VKImage.hpp"

CAM::Renderer::VKImage::VKImage(Jobs::WorkerPool* wp, Jobs::Job* /*thisJob*/, Renderer* parent, VkImage&& image)
	: parent(parent),
	wp(wp),
	device(parent->GetVKDevice()),
	vkImage(std::move(image)),
	ownImage(false)
{
	ownImage = false;
	vkImage = image;
}

CAM::Renderer::VKImage::VKImage(Jobs::WorkerPool* wp, Jobs::Job* /*thisJob*/, Renderer* parent)
	: parent(parent),
	wp(wp),
	device(parent->GetVKDevice()),
	ownImage(true)
{
	throw std::logic_error("Saddly, initing images is unimplemented.");
}

CAM::Renderer::VKImage::~VKImage()
{
	// Only destroy it if we own it
	if (ownImage)
	{
		device->deviceVKFN->vkDestroyImage((*device)(), vkImage, nullptr);
	}
}

