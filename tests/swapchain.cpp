#include <cassert>
#include <device.hpp>
#include <swapchain.hpp>

auto main() -> int
{
    const auto window = vulkan_scene::create_window("Test", 800, 600).unwrap();

    const auto instance =
        vulkan_scene::create_vulkan_instance(static_cast<bool>(false)).unwrap();
    const auto surface =
        vulkan_scene::create_surface(instance, window).unwrap();
    const auto [physical_device, graphics_queue_family, present_queue_family] =
        vulkan_scene::choose_physical_device(instance, surface).unwrap();
    const auto device =
        vulkan_scene::create_logical_device(
            physical_device, graphics_queue_family, present_queue_family
        )
            .unwrap();

    const auto swapchain =
        vulkan_scene::create_swapchain(
            device.device, physical_device, graphics_queue_family,
            present_queue_family, window, surface
        )
            .unwrap();

    assert(swapchain.images.size() > 0);
    assert(swapchain.extent.width > 0);
    assert(swapchain.extent.height > 0);

    vkDestroySwapchainKHR(device.device, swapchain.swapchain, nullptr);
    vkDestroyDevice(device.device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}
