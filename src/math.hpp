#pragma once

#include <cstddef>

// A basic linear algebra library. Contains no more than what this project
// requires.

namespace vulkan_scene
{

// Represents a Vector of any size.
template <typename T, std::size_t size> struct Vector
{
    T v[size];
};

// Different types of vectors.
using Vector2 = Vector<float, 2>;
using Vector3 = Vector<float, 3>;
using Vector4 = Vector<float, 4>;

} // namespace vulkan_scene