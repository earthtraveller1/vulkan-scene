#pragma once

#define DEBUG_PRINT(x) std::cout << #x " = " << x << std::endl

namespace vulkan_scene
{

template <kirho::printable_t... T>
auto print_error(T... args) noexcept
{
    std::cerr << "\033[91m[ERROR]: ";
    (std::cerr << ... << args) << "\033[0m\n";
}

} // namespace vulkan_scene
