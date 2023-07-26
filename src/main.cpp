#include <iostream>
#include <stdexcept>
#include <string_view>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

#define defer(name, statement) const auto name##_defer = defer {[&](){ statement; }}; (void)name##_defer

#define vk_handle_error(error, msg)\
    if (error != VK_SUCCESS)\
    {\
        throw std::runtime_error{std::string{"Failed to " msg} + std::string{". Vulkan error "} + std::to_string(error)};\
    }

constexpr uint16_t WINDOW_WIDTH = 1280;
constexpr uint16_t WINDOW_HEIGHT = 720;

// May throw an std::runtime_error.
auto create_window(std::string_view p_title, uint16_t p_width, uint16_t p_height) -> GLFWwindow*
{
    if (!glfwInit())
    {
        throw std::runtime_error{"Failed to initialize GLFW."};
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    const auto window = glfwCreateWindow(p_width, p_height, p_title.data(), nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error{"Failed to create the GLFW window."};
    }

    return window;
}

auto destroy_window(GLFWwindow* const p_window) noexcept -> void
{
    glfwDestroyWindow(p_window);
    glfwTerminate();
}

auto create_vulkan_instance(bool p_enable_validation) -> VkInstance
{
    const auto app_info = VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Vulkan Scene",
        .applicationVersion = 0,
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_2,
    };

    const auto instance_info = VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
    };

    auto instance = static_cast<VkInstance>(VK_NULL_HANDLE);
    const auto result = vkCreateInstance(&instance_info, nullptr, &instance);
    vk_handle_error(result, "create the Vulkan instance");

    return instance;
}

}

auto main() noexcept -> int
{
    try
    {
        auto window = create_window("Vulkan Scene", WINDOW_WIDTH, WINDOW_HEIGHT);
        defer(window, destroy_window(window));

        auto instance = create_vulkan_instance(false);
        defer(instance, vkDestroyInstance(instance, nullptr));

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }

        return 0;
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "\033[91m[FATAL ERROR]: " << error.what() << "\033[0m" std::endl;
        return 1;
    }
}
