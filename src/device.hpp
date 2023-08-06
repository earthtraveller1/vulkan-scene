#pragma once

#include <vulkan/vulkan.h>

namespace vulkan_scene {

struct physical_device {
  VkPhysicalDevice device;
  uint32_t graphics_family;
  uint32_t present_family;
};

auto create_vulkan_instance(bool enable_validation) noexcept
    -> kirho::result_t<VkInstance, VkResult>;

auto create_debug_messenger(VkInstance instance) noexcept
    -> kirho::result_t<VkDebugUtilsMessengerEXT, VkResult>;

auto choose_physical_device(VkInstance p_instance,
                            VkSurfaceKHR p_surface) noexcept
    -> kirho::result_t<physical_device, kirho::empty>;

auto create_logical_device(VkPhysicalDevice p_physical_device,
                           uint32_t p_graphics_family,
                           uint32_t p_present_family) noexcept
    -> kirho::result_t<VkDevice, VkResult>;

auto destroy_debug_messenger(VkInstance p_instance,
                             VkDebugUtilsMessengerEXT p_messenger) noexcept
    -> void;
} // namespace vulkan_scene
