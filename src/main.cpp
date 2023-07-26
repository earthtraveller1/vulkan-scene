#include <iostream>

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

template<typename T, typename E>
struct result
{
    result(T v): m_success(true) { m_union.value = v; }
    result(E e): m_success(false) { m_union.error = e; }

    auto is_success(T& v) const -> bool
    {
        if (m_success) v = m_union.value;
        return m_success;
    }
    
    auto is_error(E& e) const -> bool
    {
        if (!m_success) e = m_union.error;
        return m_success;
    }

    auto except(std::string_view msg) const -> T 
    {
        if (!m_success)
        {
            std::cerr << msg << '\n';
            std::abort();
        }
        else
        {
            return m_union.value;
        }
    }

    auto unwrap() const -> T
    {
        if (!m_success)
        {
            std::cerr << "[FATAL ERROR]: unwrap called on error value.\n";
            std::abort();
        }
        else
        {
            return m_union.value;
        }
    }

private:
    union error_union
    {
        T value;
        E error;
    };

    error_union m_union;
    bool m_success;
};

}

auto main() noexcept -> int
{
    std::cout << "Hello, World!\n";
    return 0;
}
