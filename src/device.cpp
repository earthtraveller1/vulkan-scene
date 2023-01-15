#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include "device.hpp"
#include "window.hpp"

using namespace std::string_literals;
using vulkan_scene::Device;

namespace
{

std::vector<const char*> get_required_instance_extensions()
{
    std::vector<const char*> extensions{VK_KHR_SURFACE_EXTENSION_NAME};

#ifdef _WIN32
    // TODO
#else
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

    return extensions;
}
} // namespace

void Device::create_instance(std::string_view p_application_name,
                             bool p_enable_validation)
{
    const VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = p_application_name.data(),
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_2};

    std::vector<const char*> layers;

    if (p_enable_validation)
    {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    const auto extensions = get_required_instance_extensions();

    const VkInstanceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()};

    const VkResult result =
        vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create the Vulkan instance. Vulkan error "s +
            std::to_string(result) + '.');
    }
}

Device::Device(std::string_view p_application_name, bool p_enable_validation,
               const Window& p_window)
{
    create_instance(p_application_name, p_enable_validation);
    m_surface = p_window.create_surface(m_instance);
}

Device::~Device() { vkDestroyInstance(m_instance, nullptr); }