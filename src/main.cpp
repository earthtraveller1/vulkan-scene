#include <iostream>

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

auto main() noexcept -> int
{
    std::cout << "Hello, World!\n";
    return 0;
}
