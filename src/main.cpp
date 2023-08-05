#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>

namespace
{

template<typename F>
concept deferable = requires(F a)
{
    { a() };
};

template<deferable F>
struct defer
{
    defer(F f) noexcept : m_f(f) {}
    ~defer() noexcept { m_f(); }
    F m_f;
};

using kirho::result_t;

#define defer(name, statement) const auto name##_defer = defer {[&]() noexcept { statement; }}; (void)name##_defer;

#define vk_handle_error(error, msg) {\
    if (error != VK_SUCCESS)\
    {\
        throw std::runtime_error{std::string{"Failed to " msg} + std::string{". Vulkan error "} + std::to_string(error)};\
    }\
}\

template<kirho::printable... T>
auto print_error(T... args) noexcept
{
    std::cerr << "\033[91m[ERROR]: "; 
    (std::cerr << ... << args) << "\033[0m\n";
}

constexpr uint16_t WINDOW_WIDTH = 1280;
constexpr uint16_t WINDOW_HEIGHT = 720;

const auto MESSENGER_CREATE_INFO = VkDebugUtilsMessengerCreateInfoEXT {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = 
        [](
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
            VkDebugUtilsMessageTypeFlagsEXT messageTypes, 
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
            void* pUserData
        ) noexcept -> VkBool32
        {
            switch (messageSeverity)
            {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                    std::cout << "\033[90m[VULKAN]: ";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                    std::cout << "\033[90m[VULKAN]: ";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                    std::cerr << "\033[93m[VULKAN]: ";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                    std::cerr << "\033[91m[VULKAN]: ";
                    break;
                default:
                    std::cout << "[VULKAN]: ";
                    break;
            }

            std::cout << pCallbackData->pMessage << "\033[0m\n";
    
            return VK_FALSE;
        },
    .pUserData = nullptr,
};

const auto DEVICE_EXTENSIONS = std::array<const char*, 1> { "VK_KHR_swapchain" };

// May throw an std::runtime_error.
auto create_window(std::string_view p_title, uint16_t p_width, uint16_t p_height) noexcept -> result_t<GLFWwindow*, const char*>
{
    using result_t_t = result_t<GLFWwindow*, const char*>;

    if (!glfwInit())
    {
        return result_t_t::error("[ERROR]: Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    const auto window = glfwCreateWindow(p_width, p_height, p_title.data(), nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return result_t_t::error("Failed to create the GLFW window.");
    }

    return result_t_t::success(window);
}

auto destroy_window(GLFWwindow* const p_window) noexcept -> void
{
    glfwDestroyWindow(p_window);
    glfwTerminate();
}

auto create_vulkan_instance(bool p_enable_validation) noexcept -> result_t<VkInstance, VkResult>
{
    using result_t_t = result_t<VkInstance, VkResult>;

    const auto app_info = VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Vulkan Scene",
        .applicationVersion = 0,
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_2,
    };

    auto glfw_extension_count = static_cast<uint32_t>(0);
    const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    auto enabled_layers = std::vector<const char*>();
    auto enabled_extensions = std::vector<const char*>(glfw_extensions, glfw_extensions + glfw_extension_count);

    if (p_enable_validation)
    {
        constexpr auto VK_LAYER_KHRONOS_validation = "VK_LAYER_KHRONOS_validation";
    
        auto available_layer_count = static_cast<uint32_t>(0);
        vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
    
        auto available_layers = std::vector<VkLayerProperties>(available_layer_count);
        vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data());

        auto layer_available = false;

        for (const auto& layer : available_layers)
        {
            if (std::strcmp(layer.layerName, VK_LAYER_KHRONOS_validation) == 0)
            {
                layer_available = true;
            }
        }

        if (!layer_available)
        {
            print_error("Validation layers requested, but are not available.");
            return result_t_t::error(VK_ERROR_LAYER_NOT_PRESENT);
        }

        enabled_layers.push_back(VK_LAYER_KHRONOS_validation);
        
        // We don't need to check for this extension's existence, as it should
        // exists along with the validation layers.
        enabled_extensions.push_back("VK_EXT_debug_utils");
    }

    const auto instance_info = VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = p_enable_validation ? &MESSENGER_CREATE_INFO : nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
        .ppEnabledLayerNames = enabled_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
        .ppEnabledExtensionNames = enabled_extensions.data(),
    };

    auto instance = static_cast<VkInstance>(VK_NULL_HANDLE);
    const auto result_t = vkCreateInstance(&instance_info, nullptr, &instance);
    if (result_t != VK_SUCCESS)
    {
        std::cerr << "[error]: failed to create instance vulkan error " << result_t << '\n';
        return result_t_t::error(result_t);
    }

    return result_t_t::success(instance);
}

auto create_surface(VkInstance p_instance, GLFWwindow* p_window) noexcept -> result_t<VkSurfaceKHR, VkResult>
{
    using result_t_t = result_t<VkSurfaceKHR, VkResult>;

    auto surface = static_cast<VkSurfaceKHR>(VK_NULL_HANDLE);
    const auto result_t = glfwCreateWindowSurface(p_instance, p_window, nullptr, &surface);
    if (result_t != VK_SUCCESS)
    {
        print_error("Failed to create the window surface. Vulkan error ", result_t, ".");
        return result_t_t::error(result_t);
    }

    return result_t_t::success(surface);
}

struct physical_device
{
    VkPhysicalDevice device;
    uint32_t graphics_family;
    uint32_t present_family;
};

auto choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) noexcept -> result_t<physical_device, kirho::empty>
{
    using result_t_t = result_t<physical_device, kirho::empty>;

    auto device_count = static_cast<uint32_t>(0);
    vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

    if (device_count == 0)
    {
        print_error("There appears to be no devices on this system that supports Vulkan.");
        return result_t_t::error(kirho::empty{});
    }

    auto physical_devices = std::vector<VkPhysicalDevice>(device_count);
    vkEnumeratePhysicalDevices(p_instance, &device_count, physical_devices.data());

    auto chosen_device = static_cast<VkPhysicalDevice>(VK_NULL_HANDLE);
    auto graphics_family = std::optional<uint32_t>();
    auto present_family = std::optional<uint32_t>();

    for (const auto device : physical_devices)
    {
        auto queue_family_count = static_cast<uint32_t>(0);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

        auto queue_families = std::vector<VkQueueFamilyProperties>(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

        for (decltype(queue_families.size()) i = 0; i < queue_families.size(); i++)
        {
            if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                graphics_family = static_cast<uint32_t>(i);
            }

            auto present_support = static_cast<VkBool32>(VK_FALSE);
            vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(i), p_surface, &present_support);
            if (present_support == VK_TRUE)
            {
                present_family = static_cast<uint32_t>(i);
            }

            auto extension_count = static_cast<uint32_t>(0);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

            auto available_extensions = std::vector<VkExtensionProperties>(extension_count);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

            auto found_unsupported_extension = false;

            for (const auto extension : DEVICE_EXTENSIONS)
            {
                auto found_extension = false;

                for (const auto& available_extension : available_extensions)
                {
                    if (std::strcmp(available_extension.extensionName, extension) == 0)
                    {
                        found_extension = true;
                        break;
                    }
                }

                if (!found_extension)
                {
                    found_unsupported_extension = true;
                    break;
                }
            }

            if (graphics_family.has_value() && present_family.has_value() && !found_unsupported_extension)
            {
                break;
            }
        }

        if (graphics_family.has_value() && present_family.has_value())
        {
            chosen_device = device;
            break;
        }
    }

    if (!graphics_family.has_value())
    {
        print_error("Could not find a graphics queue family on any devices on this system.");
        return result_t_t::error(kirho::empty{});
    }

    if (!present_family.has_value())
    {
        print_error("Could not find a present queue family on any devices on this system.");
        return result_t_t::error(kirho::empty{});
    }

    if (chosen_device == VK_NULL_HANDLE)
    {
        print_error("Could not find an adequate physical device on this system.");
        return result_t_t::error(kirho::empty{});
    }

    auto device_properties = VkPhysicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(chosen_device, &device_properties);

    std::cout << "[INFO]: Selected the " << device_properties.deviceName << " graphics card.\n";

    return result_t_t::success(physical_device{chosen_device, graphics_family.value(), present_family.value()});
}

auto create_debug_messenger(VkInstance p_instance) noexcept -> result_t<VkDebugUtilsMessengerEXT, VkResult>
{
    using result_t_t = result_t<VkDebugUtilsMessengerEXT, VkResult>;

    const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(p_instance, "vkCreateDebugUtilsMessengerEXT"));
    if (function == nullptr)
    {
        print_error("Cannot load vkCreateDebugUtilsMessengerEXT.");
        return result_t_t::error(VK_ERROR_EXTENSION_NOT_PRESENT);
    }

    auto messenger = static_cast<VkDebugUtilsMessengerEXT>(VK_NULL_HANDLE);
    const auto result_t = function(p_instance, &MESSENGER_CREATE_INFO, nullptr, &messenger);
    if (result_t != VK_SUCCESS)
    {
        print_error("Failed to create the debug messenger. Vulkan error ", result_t, ".");
        return result_t_t::error(result_t);
    }

    return result_t_t::success(messenger);
}

auto create_logical_device(VkPhysicalDevice p_physical_device, uint32_t p_graphics_family, uint32_t p_present_family) noexcept -> result_t<VkDevice, VkResult>
{
    using result_t_t = result_t<VkDevice, VkResult>;

    auto queue_infos = std::vector<VkDeviceQueueCreateInfo>();
    const auto queue_priority = 1.0f;

    if (p_graphics_family == p_present_family)
    {
        queue_infos.push_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = p_graphics_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        });
    }
    else
    {
        auto queue_info = VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = p_graphics_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        queue_infos.push_back(queue_info);
        queue_info.queueFamilyIndex = p_present_family;
        queue_infos.push_back(queue_info);
    }

    const auto device_info = VkDeviceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
        .pQueueCreateInfos = queue_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
        .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
        .pEnabledFeatures = nullptr,
    };

    auto device = static_cast<VkDevice>(VK_NULL_HANDLE);
    const auto result_t = vkCreateDevice(p_physical_device, &device_info, nullptr, &device);
    if (result_t != VK_SUCCESS)
    {
        print_error("Failed to create the logical device. Vulkan error ", result_t, ".");
        return result_t_t::error(result_t);
    }

    return result_t_t::success(device);
}

auto create_swapchain(
        VkDevice p_device, 
        VkPhysicalDevice p_physical_device, 
        uint32_t p_graphics_family, 
        uint32_t p_present_family, 
        GLFWwindow* p_window, 
        VkSurfaceKHR p_surface
) noexcept -> result_t<VkSwapchainKHR, VkResult>
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_physical_device, p_surface, &surface_capabilities);

    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface, &surface_format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface, &surface_format_count, surface_formats.data());

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface, &present_mode_count, nullptr);

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface, &present_mode_count, present_modes.data());

    VkSurfaceFormatKHR surface_format = surface_formats.front();

    for (const auto& format : surface_formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format = format;
            break;
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (auto mode : present_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = mode;
            break;
        }
    }

    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(p_window, &framebuffer_width, &framebuffer_height);

    const VkExtent2D swap_extent = 
        surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() 
            ? surface_capabilities.currentExtent 
            : VkExtent2D {
                std::clamp(static_cast<uint32_t>(framebuffer_width), surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
                std::clamp(static_cast<uint32_t>(framebuffer_height), surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height),
            };

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    const auto has_max_image_count = surface_capabilities.maxImageCount > 0;
    if (has_max_image_count && image_count > surface_capabilities.maxImageCount)
    {
        image_count = surface_capabilities.maxImageCount;
    }

    const auto queue_families_same = p_graphics_family == p_present_family;

    std::array<uint32_t, 2> queue_families{p_graphics_family, p_present_family};

    const VkSwapchainCreateInfoKHR swapchain_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = p_surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = swap_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = queue_families_same ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = queue_families_same ? 0 : static_cast<uint32_t>(queue_families.size()),
        .pQueueFamilyIndices = queue_families_same ? nullptr : queue_families.data(),
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkSwapchainKHR swapchain;

    using result_t_t = result_t<VkSwapchainKHR, VkResult>;

    const auto result_t = vkCreateSwapchainKHR(p_device, &swapchain_info, nullptr, &swapchain);
    if (result_t != VK_SUCCESS)
    {
        print_error("Failed to create the swapchain. Vulkan error ", result_t);
        return result_t_t::error(result_t);
    }

    return result_t_t::success(swapchain);
}

auto destroy_debug_messenger(VkInstance p_instance, VkDebugUtilsMessengerEXT p_messenger) noexcept
{
    const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(p_instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (function != nullptr)
    {
        function(p_instance, p_messenger, nullptr);
    }

    // Nothing we can do if function turned out to be null.
}

}

auto main() noexcept -> int
{
    const auto enable_validation = true;

    const auto window = create_window("Vulkan Scene", WINDOW_WIDTH, WINDOW_HEIGHT).unwrap();
    defer(window, destroy_window(window));

    const auto instance = create_vulkan_instance(enable_validation).unwrap();
    defer(instance, vkDestroyInstance(instance, nullptr));

    const auto debug_messenger = enable_validation ? create_debug_messenger(instance).unwrap() : VK_NULL_HANDLE;
    defer(debug_messenger, enable_validation ? destroy_debug_messenger(instance, debug_messenger) : (void)0);

    const auto surface = create_surface(instance, window).unwrap();
    defer(surface, vkDestroySurfaceKHR(instance, surface, nullptr));

    const auto [physical_device, graphics_queue_family, present_queue_family] = choose_physical_device(instance, surface).unwrap();

    const auto logical_device = create_logical_device(physical_device, graphics_queue_family, present_queue_family).unwrap();
    defer(logical_device, vkDestroyDevice(logical_device, nullptr));

    const auto swapchain = create_swapchain(logical_device, physical_device, graphics_queue_family, present_queue_family, window, surface).unwrap();
    defer(swapchain, vkDestroySwapchainKHR(logical_device, swapchain, nullptr));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    return 0;
}
