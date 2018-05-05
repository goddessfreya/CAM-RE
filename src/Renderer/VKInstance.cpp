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

#include "VKInstance.hpp"
#include "Renderer.hpp"
#include "../Utils/VersionNumber.hpp"

CAM::Renderer::VKInstance::VKInstance
(
	Jobs::WorkerPool* wp,
	Jobs::Job* /*thisJob*/,
	Renderer* parent
) : wp(wp), parent(parent)
{
	if constexpr(Config::ValidationEnabled)
	{
		layers =
		{
			"VK_LAYER_LUNARG_standard_validation"
		};
	}

	exts = {};

	if constexpr(Config::ValidationEnabled)
	{
		exts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	auto sdlReqExts = parent->GetSDLWindow()->GetReqExts();
	exts.insert(std::end(exts), std::begin(sdlReqExts), std::end(sdlReqExts));

	uint32_t layerCount;
	VKFNCHECKRETURN(CAM::VKFN::vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
	std::vector<VkLayerProperties> availableLayers(layerCount);
	VKFNCHECKRETURN(CAM::VKFN::vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

	printf("Available Layers:\n");
	for (auto& layer : availableLayers)
	{
		printf("\t%s\n", layer.layerName);
	}

	printf("Requested Layers:\n");
	for (auto& layer : layers)
	{
		printf("\t%s\n", layer);
		bool found = false;
		for (auto& aLayer : availableLayers)
		{
			if (std::strcmp(aLayer.layerName, layer))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			throw std::runtime_error("Couldn't find layer " + std::string(layer));
		}
	}

	uint32_t extCount;
	VKFNCHECKRETURN(CAM::VKFN::vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr));
	std::vector<VkExtensionProperties> availableExts(extCount);
	VKFNCHECKRETURN(CAM::VKFN::vkEnumerateInstanceExtensionProperties(nullptr, &extCount, availableExts.data()));

	printf("Available Exts:\n");
	for (auto& ext : availableExts)
	{
		printf("\t%s\n", ext.extensionName);
	}

	printf("Requested Exts:\n");
	for (auto& ext : exts)
	{
		printf("\t%s\n", ext);
		bool found = false;
		for (auto& aExt : availableExts)
		{
			if (std::strcmp(aExt.extensionName, ext))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			throw std::runtime_error("Couldn't find ext " + std::string(ext));
		}
	}

	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "CAM-RE";
	appInfo.applicationVersion = Version::commitNumber;
	appInfo.pEngineName = nullptr;
	appInfo.engineVersion = 0;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = layers.size();
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.enabledExtensionCount = exts.size();
	createInfo.ppEnabledExtensionNames = exts.data();
	VKFNCHECKRETURN(CAM::VKFN::vkCreateInstance(&createInfo, nullptr, &instance));

	instanceVKFN = std::make_unique<InstanceVKFN>(&instance);
	if (!instanceVKFN->InitInstanceFuncs()) { throw std::runtime_error("Could not init instance funcs\n"); }

	if constexpr (Config::ValidationEnabled)
	{
		VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
		callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		callbackCreateInfo.pNext = nullptr;
		callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		callbackCreateInfo.pfnCallback = &VKInstance::DebugCallback;
		callbackCreateInfo.pUserData = (void*)this;

		VKFNCHECKRETURN(instanceVKFN->vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &dcb));
	}
}

CAM::Renderer::VKInstance::~VKInstance()
{
	if constexpr (Config::ValidationEnabled)
	{
		instanceVKFN->vkDestroyDebugReportCallbackEXT(instance, dcb, nullptr);
	}
	instanceVKFN->vkDestroyInstance(instance, nullptr);
}

VkBool32 VKAPI_PTR CAM::Renderer::VKInstance::DebugCallback
(
	VkDebugReportFlagsEXT /*flags*/,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* /*pUserData*/
)
{
	printf("%s: %lu (%i): Loc: %zu Code: %u: %s\n", pLayerPrefix, object, objectType, location, messageCode, pMessage);

	return VK_FALSE;
}
