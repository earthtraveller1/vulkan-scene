#pragma once

#include <cstddef>
#include <concepts>

// A basic linear algebra library. Contains no more than what this project
// requires.

namespace vulkan_scene
{

// No support for integer values for now.
template <typename T>
concept Scalar = requires(T a, T b)
{
    { a + b };
    { a * b };
    { a / b };
    { a - b };
};

// Represents a Vector of any size.
template <Scalar T, std::size_t Size> struct Vector
{
    T v[Size];
};

// Different types of vectors.
template<typename T>
using Vector2 = Vector<T, 2>;
template<typename T>
using Vector3 = Vector<T, 3>;
template<typename T>
using Vector4 = Vector<T, 4>;

// Vectors that uses floats
using Vector2f = Vector<float, 2>;
using Vector3f = Vector<float, 3>;
using Vector4f = Vector<float, 4>;

} // namespace vulkan_scene