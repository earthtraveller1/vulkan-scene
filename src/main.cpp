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

#define defer(name, statement) const auto name##_defer = defer {[](){ statement; }}; (void)name##_defer

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

}

auto main() noexcept -> int
{
    std::cout << "Hello, World!\n";
    return 0;
}
