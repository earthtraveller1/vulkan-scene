#pragma once

#include <device.hpp>
#include <window.hpp>

auto test_devices(bool p_enable_validation) noexcept -> void
{
    const auto window = vulkan_scene::create_window("Test", 800, 600).unwrap();

    const auto instance = vulkan_scene::create_vulkan_instance(
                              static_cast<bool>(p_enable_validation)
    )
                              .unwrap();
    const auto surface =
        vulkan_scene::create_surface(instance, window).unwrap();
    const auto [physical_device, graphics_queue_family, present_queue_family] =
        vulkan_scene::choose_physical_device(instance, surface).unwrap();
    const auto device =
        vulkan_scene::create_logical_device(
            physical_device, graphics_queue_family, present_queue_family
        )
            .unwrap();

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}
