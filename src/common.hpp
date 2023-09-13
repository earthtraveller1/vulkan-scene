#pragma once

#define DEBUG_PRINT(x) std::cout << #x " = " << x << std::endl

namespace vulkan_scene
{

template <typename T>
concept iterable_c = requires(T a) {
    {
        a.begin()
    };
    {
        a.end()
    };
};

template <kirho::printable_t... T>
auto print_error(T... args) noexcept
{
    std::cerr << "\033[91m[ERROR]: ";
    (std::cerr << ... << args) << "\033[0m\n";
}

template <iterable_c T>
auto operator<<(std::ostream& p_stream, T p_container) -> std::ostream&
{
    p_stream << "[ ";

    for (const auto& value : p_container)
    {
        p_stream << value << ", ";
    }

    p_stream << " ]";
}

} // namespace vulkan_scene
