#pragma once

namespace vulkan_scene {

template <kirho::printable... T> auto print_error(T... args) noexcept {
  std::cerr << "\033[91m[ERROR]: ";
  (std::cerr << ... << args) << "\033[0m\n";
}

}
