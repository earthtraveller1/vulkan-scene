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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    return 0;
}
