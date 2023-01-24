#include <optional>
#include <set>

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
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

    return extensions;
}

const char* const DEVICE_EXTENSIONS[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// Retrieves the queue families of a physical device. The first value in the
// return tuple is the graphics family, with the second one being the present
// family.
std::tuple<std::optional<uint32_t>, std::optional<uint32_t>>
get_queue_families(VkPhysicalDevice p_device, VkSurfaceKHR p_surface)
{
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(p_device, &queue_family_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(p_device, &queue_family_count,
                                             queue_families.data());

    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    for (uint32_t i = 0; i < queue_families.size(); i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_family = i;
        }

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(p_device, i, p_surface,
                                             &present_support);

        if (present_support)
        {
            present_family = i;
        }
    }

    return {graphics_family, present_family};
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

VkCommandBuffer Device::allocate_primary_cmd_buffer() const
{
    const VkCommandBufferAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    VkCommandBuffer cmd_buffer;
    const auto result =
        vkAllocateCommandBuffers(m_device, &allocate_info, &cmd_buffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to allocate a primary command buffer. Vulkan error "s +
            std::to_string(result) + '.');
    }
    
    return cmd_buffer;
}

Device::Device(std::string_view p_application_name, bool p_enable_validation,
               const Window& p_window)
{
    create_instance(p_application_name, p_enable_validation);
    m_surface = p_window.create_surface(m_instance);
    choose_physical_device();
    create_logical_device();
    create_command_pool();
}

Device::~Device() { vkDestroyInstance(m_instance, nullptr); }

void Device::choose_physical_device()
{
    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(m_instance, &physical_device_count,
                               physical_devices.data());

    VkPhysicalDevice chosen_device = VK_NULL_HANDLE;

    for (const auto device : physical_devices)
    {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);

        // We don't want to use any software implementations of Vulkan.
        if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
        {
            continue;
        }

        const auto [graphics_family, present_family] =
            get_queue_families(device, m_surface);

        if (graphics_family.has_value() && present_family.has_value())
        {
            // After ensuring that the device has the required queue families,
            // we then check if it has the required extensions.

            uint32_t extension_count;
            vkEnumerateDeviceExtensionProperties(device, nullptr,
                                                 &extension_count, nullptr);

            std::vector<VkExtensionProperties> extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(
                device, nullptr, &extension_count, extensions.data());

            std::set<std::string> required_extensions(DEVICE_EXTENSIONS,
                                                      DEVICE_EXTENSIONS + 1);

            for (const auto& extension : extensions)
            {
                required_extensions.erase(extension.extensionName);
            }

            if (required_extensions.empty())
            {

                // We chose the first physical device that satisfies our
                // requirements.
                chosen_device = device;
                break;
            }
        }
    }

    if (chosen_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find an adequate physical device.");
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(chosen_device, &device_properties);

    std::cout << "[INFO]: Selected the " << device_properties.deviceName
              << " graphics card.\n";

    m_physical_device = chosen_device;
}

void Device::create_logical_device()
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    const std::set<uint32_t> queue_families{m_graphics_queue_family,
                                            m_present_queue_family};
    const float queue_priority = 1.0f;

    for (const auto family : queue_families)
    {
        queue_create_infos.push_back(
            {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .pNext = nullptr,
             .flags = 0,
             .queueFamilyIndex = family,
             .queueCount = 1,
             .pQueuePriorities = &queue_priority});
    }

    const VkDeviceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount =
            static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = DEVICE_EXTENSIONS,
        .pEnabledFeatures = nullptr};

    const auto result =
        vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create the logical device. Vulkan error "s +
            std::to_string(result) + '.');
    }

    vkGetDeviceQueue(m_device, m_graphics_queue_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_present_queue_family, 0, &m_present_queue);
}

void Device::create_command_pool()
{
    const VkCommandPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_graphics_queue_family};

    const auto result =
        vkCreateCommandPool(m_device, &create_info, nullptr, &m_command_pool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create the command pool. Vulkan error "s +
            std::to_string(result) + '.');
    }
}