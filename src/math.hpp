#pragma once

#include <concepts>
#include <cstddef>

// A basic linear algebra library. Contains no more than what this project
// requires.

namespace vulkan_scene
{

// No support for integer values for now.
template <typename T>
concept Scalar = requires(T a, T b) {
                     {
                         a + b
                     };
                     {
                         a* b
                     };
                     {
                         a / b
                     };
                     {
                         a - b
                     };
                 };

// Represents a Vector of any size.
template <Scalar T, std::size_t Size> struct Vector
{
    T v[Size];
};

// Utility macro for shortening code.
#define MAKE_MEMBER(name, index)                                               \
    inline T& name()                                                           \
    {                                                                          \
        return v[index];                                                       \
    }                                                                          \
    inline T name() const                                                      \
    {                                                                          \
        return v[index];                                                       \
    }

// Two-component vector
template <typename T> struct Vector<T, 2>
{
    MAKE_MEMBER(x, 0)
    MAKE_MEMBER(y, 1)
};

// Three-component vector
template <typename T> struct Vector<T, 3>
{
    MAKE_MEMBER(x, 0)
    MAKE_MEMBER(y, 1)
    MAKE_MEMBER(z, 2)
};

// Four component vector
template <typename T> struct Vector<T, 4>
{
    MAKE_MEMBER(x, 0)
    MAKE_MEMBER(y, 1)
    MAKE_MEMBER(z, 2)
    MAKE_MEMBER(w, 3)
};

#undef MAKE_MEMBER

// Different types of vectors.
template <typename T> using Vector2 = Vector<T, 2>;
template <typename T> using Vector3 = Vector<T, 3>;
template <typename T> using Vector4 = Vector<T, 4>;

// Vectors that uses floats
using Vector2f = Vector<float, 2>;
using Vector3f = Vector<float, 3>;
using Vector4f = Vector<float, 4>;

} // namespace vulkan_scene