#pragma once

#include <vulkan/vulkan.h>

namespace vulkan_scene
{

struct physical_device
{
    VkPhysicalDevice device;
    uint32_t graphics_family;
    uint32_t present_family;
};

struct logical_device
{
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
};

auto create_vulkan_instance(bool enable_validation) noexcept
    -> kirho::result_t<VkInstance, VkResult>;

auto create_debug_messenger(VkInstance instance) noexcept
    -> kirho::result_t<VkDebugUtilsMessengerEXT, VkResult>;

auto choose_physical_device(
    VkInstance p_instance, VkSurfaceKHR p_surface
) noexcept -> kirho::result_t<physical_device, kirho::empty>;

auto create_logical_device(
    VkPhysicalDevice p_physical_device,
    uint32_t p_graphics_family,
    uint32_t p_present_family
) noexcept -> kirho::result_t<logical_device, VkResult>;

auto destroy_debug_messenger(
    VkInstance p_instance, VkDebugUtilsMessengerEXT p_messenger
) noexcept -> void;

auto create_command_pool(VkDevice p_device, uint32_t p_queue_family) noexcept
    -> kirho::result_t<VkCommandPool, VkResult>;

auto create_semaphore(VkDevice p_device) noexcept
    -> kirho::result_t<VkSemaphore, VkResult>;

auto create_fence(VkDevice p_device) noexcept
    -> kirho::result_t<VkFence, VkResult>;

auto create_command_buffer(VkDevice p_device, VkCommandPool p_pool) noexcept
    -> kirho::result_t<VkCommandBuffer, VkResult>;

} // namespace vulkan_scene
