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

#include "SDLWindow.hpp"
#include "Renderer.hpp"
#include "VKSurface.hpp"

CAM::Renderer::VKSurface::VKSurface(Jobs::WorkerPool* wp, Jobs::Job* /*thisJob*/, Renderer* parent)
	: wp(wp),
	parent(parent),
	instance(parent->GetVKInstance()),
	window(parent->GetSDLWindow())
{
	if (!SDL_Vulkan_CreateSurface((*window)(), (*instance)(), &vkSurface))
	{
		throw std::runtime_error("Could not create surface");
	}
}

CAM::Renderer::VKSurface::~VKSurface()
{
	instance->instanceVKFN->vkDestroySurfaceKHR((*instance)(), vkSurface, nullptr);
}
