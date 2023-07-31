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
    defer(F f): m_f(f) {}
    ~defer() { m_f(); }
    F m_f;
};

using kirho::result;

#define defer(name, statement) const auto name##_defer = defer {[&](){ statement; }}; (void)name##_defer;

#define vk_handle_error(error, msg) {\
    if (error != VK_SUCCESS)\
    {\
        throw std::runtime_error{std::string{"Failed to " msg} + std::string{". Vulkan error "} + std::to_string(error)};\
    }\
}\

template<kirho::printable... T>
auto print_error(T... args)
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
        ) -> VkBool32
        {
            std::cout << "[VULKAN]: " << pCallbackData->pMessage << '\n';
    
            return VK_FALSE;
        },
    .pUserData = nullptr,
};

const auto DEVICE_EXTENSIONS = std::array<const char*, 1> { "VK_KHR_swapchain" };

// May throw an std::runtime_error.
auto create_window(std::string_view p_title, uint16_t p_width, uint16_t p_height) noexcept -> result<GLFWwindow*, const char*>
{
    using result_t = result<GLFWwindow*, const char*>;

    if (!glfwInit())
    {
        return result_t::error("[ERROR]: Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    const auto window = glfwCreateWindow(p_width, p_height, p_title.data(), nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return result_t::error("Failed to create the GLFW window.");
    }

    return result_t::success(window);
}

auto destroy_window(GLFWwindow* const p_window) noexcept -> void
{
    glfwDestroyWindow(p_window);
    glfwTerminate();
}

auto create_vulkan_instance(bool p_enable_validation) noexcept -> result<VkInstance, VkResult>
{
    using result_t = result<VkInstance, VkResult>;

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
            return result_t::error(VK_ERROR_LAYER_NOT_PRESENT);
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
    const auto result = vkCreateInstance(&instance_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[error]: failed to create instance vulkan error " << result << '\n';
        return result_t::error(result);
    }

    return result_t::success(instance);
}

auto create_surface(VkInstance p_instance, GLFWwindow* p_window) -> result<VkSurfaceKHR, VkResult>
{
    using result_t = result<VkSurfaceKHR, VkResult>;

    auto surface = static_cast<VkSurfaceKHR>(VK_NULL_HANDLE);
    const auto result = glfwCreateWindowSurface(p_instance, p_window, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        print_error("Failed to create the window surface. Vulkan error ", result, ".");
        return result_t::error(result);
    }

    return result_t::success(surface);
}

struct physical_device
{
    VkPhysicalDevice device;
    uint32_t graphics_family;
    uint32_t present_family;
};

auto choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) -> result<physical_device, kirho::empty>
{
    using result_t = result<physical_device, kirho::empty>;

    auto device_count = static_cast<uint32_t>(0);
    vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

    if (device_count == 0)
    {
        print_error("There appears to be no devices on this system that supports Vulkan.");
        return result_t::error(kirho::empty{});
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
        return result_t::error(kirho::empty{});
    }

    if (!present_family.has_value())
    {
        print_error("Could not find a present queue family on any devices on this system.");
        return result_t::error(kirho::empty{});
    }

    if (chosen_device == VK_NULL_HANDLE)
    {
        print_error("Could not find an adequate physical device on this system.");
        return result_t::error(kirho::empty{});
    }

    auto device_properties = VkPhysicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(chosen_device, &device_properties);

    std::cout << "[INFO]: Selected the " << device_properties.deviceName << " graphics card.\n";

    return result_t::success(physical_device{chosen_device, graphics_family.value(), present_family.value()});
}

auto create_debug_messenger(VkInstance p_instance) -> result<VkDebugUtilsMessengerEXT, VkResult>
{
    using result_t = result<VkDebugUtilsMessengerEXT, VkResult>;

    const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(p_instance, "vkCreateDebugUtilsMessengerEXT"));
    if (function == nullptr)
    {
        print_error("Cannot load vkCreateDebugUtilsMessengerEXT.");
        return result_t::error(VK_ERROR_EXTENSION_NOT_PRESENT);
    }

    auto messenger = static_cast<VkDebugUtilsMessengerEXT>(VK_NULL_HANDLE);
    const auto result = function(p_instance, &MESSENGER_CREATE_INFO, nullptr, &messenger);
    if (result != VK_SUCCESS)
    {
        print_error("Failed to create the debug messenger. Vulkan error ", result, ".");
        return result_t::error(result);
    }

    return result_t::success(messenger);
}

auto create_logical_device(VkPhysicalDevice p_physical_device, uint32_t p_graphics_family, uint32_t p_present_family) -> result<VkDevice, VkResult>
{
    using result_t = result<VkDevice, VkResult>;

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
    const auto result = vkCreateDevice(p_physical_device, &device_info, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        print_error("Failed to create the logical device. Vulkan error ", result, ".");
        return result_t::error(result);
    }

    return result_t::success(device);
}

auto destroy_debug_messenger(VkInstance p_instance, VkDebugUtilsMessengerEXT p_messenger)
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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    return 0;
}
