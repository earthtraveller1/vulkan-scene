#include <device.hpp>
#include <window.hpp>

auto main() -> int
{
    // Without validation layers.
    const auto instance1 = vulkan_scene::create_vulkan_instance(false).unwrap();
    vkDestroyInstance(instance1, nullptr);

    // With validation layres.
    const auto instance2 = vulkan_scene::create_vulkan_instance(true).unwrap();
    vkDestroyInstance(instance2, nullptr);
}
