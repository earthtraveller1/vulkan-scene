#include <iostream>

template<typename F>
struct defer
{
    defer(F f): m_f(f) {}
    ~defer() { m_f(); }
    F m_f;
};

#define defer(name, statement) const auto name##_defer = defer {[](){ statement; }}; (void)name##_defer

auto main() -> int
{
    std::cout << "Hello, World!\n";
    return 0;
}
