#ifndef INCLUDED_VK_EXT_H
#define INCLUDED_VK_EXT_H

#include <vulkan/vulkan.h>

/* Vulkan extension functions that are not loaded by default. */

VkResult earthtraveller1_vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
#define vkCreateDebugUtilsMessengerEXT earthtraveller1_vkCreateDebugUtilsMessengerEXT

void earthtraveller1_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);
#define vkDestroyDebugUtilsMessengerEXT earthtraveller1_vkDestroyDebugUtilsMessengerEXT

#endif