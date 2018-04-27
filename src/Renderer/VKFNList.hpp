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

#ifdef VKFNGLOBALPROC
	VKFNGLOBALPROC(vkEnumerateInstanceExtensionProperties)
	VKFNGLOBALPROC(vkEnumerateInstanceLayerProperties)
	VKFNGLOBALPROC(vkCreateInstance)
#endif

#ifdef VKFNINSTANCEPROC
	VKFNINSTANCEPROC(vkDestroyInstance)
	VKFNINSTANCEPROC(vkDestroySurfaceKHR)
	VKFNINSTANCEPROC(vkEnumeratePhysicalDevices)
	VKFNINSTANCEPROC(vkGetPhysicalDeviceProperties)
	VKFNINSTANCEPROC(vkGetPhysicalDeviceQueueFamilyProperties)
	VKFNINSTANCEPROC(vkGetPhysicalDeviceFeatures)
	VKFNINSTANCEPROC(vkCreateDevice)
	VKFNINSTANCEPROC(vkDestroyDevice)
	VKFNINSTANCEPROC(vkGetDeviceProcAddr)
	VKFNINSTANCEPROC(vkEnumerateDeviceExtensionProperties)
	VKFNINSTANCEPROC(vkGetPhysicalDeviceSurfaceSupportKHR)
#endif

#ifdef VKFNINSTANCEPROC_VAL
	VKFNINSTANCEPROC_VAL(vkCreateDebugReportCallbackEXT)
	VKFNINSTANCEPROC_VAL(vkDestroyDebugReportCallbackEXT)
#endif

#ifdef VKFNDEVICEPROC
	VKFNDEVICEPROC(vkDeviceWaitIdle)
	VKFNDEVICEPROC(vkGetDeviceQueue)
#endif
