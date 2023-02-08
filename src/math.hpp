#pragma once

#include <concepts>
#include <cstddef>

// A basic linear algebra library. Contains no more than what this project
// requires.

namespace vulkan_scene
{

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
    T v[2];
    
    inline Vector(T x = 0, T y = 0)
    {
        v[0] = x;
        v[1] = y;
    }

    MAKE_MEMBER(x, 0)
    MAKE_MEMBER(y, 1)
};

// Three-component vector
template <typename T> struct Vector<T, 3>
{
    T v[3];
    
    inline Vector(T x = 0, T y = 0, T z = 0)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }

    MAKE_MEMBER(x, 0)
    MAKE_MEMBER(y, 1)
    MAKE_MEMBER(z, 2)
};

// Four component vector
template <typename T> struct Vector<T, 4>
{
    T v[4];
    
    inline Vector(T x = 0, T y = 0, T z = 0, T w = 0)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = w;
    }

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
using Vector2f = Vector2<float>;
using Vector3f = Vector3<float>;
using Vector4f = Vector4<float>;

} // namespace vulkan_scene