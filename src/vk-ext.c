#include "vk-ext.h"

#define LOAD_FUNC(func, name)                                                  \
    PFN_##func name = (PFN_##func)vkGetInstanceProcAddr(instance, #func)

VkResult earthtraveller1_vkCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger)
{
    LOAD_FUNC(vkCreateDebugUtilsMessengerEXT, func);
    if (func != NULL)
    {
        return func(instance, pCreateInfo, pAllocator, pMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void earthtraveller1_vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* pAllocator)
{
    LOAD_FUNC(vkDestroyDebugUtilsMessengerEXT, func);
    if (func != NULL)
    {
        func(instance, messenger, pAllocator);
    }
}
