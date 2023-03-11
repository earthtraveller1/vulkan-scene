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

    Vector<T, Size> operator+(const Vector<T, Size>& b) const
    {
        Vector result;

        for (size_t i = 0; i < Size; i++)
        {
            result.v[i] = v[i] + b.v[i];
        }

        return result;
    }

    Vector<T, Size> operator-(const Vector<T, Size>& b) const
    {
        Vector result;

        for (size_t i = 0; i < Size; i++)
        {
            result.v[i] = v[i] - b.v[i];
        }

        return result;
    }

    Vector<T, Size> operator*(T b) const
    {
        Vector result;

        for (size_t i = 0; i < Size; i++)
        {
            result.v[i] = v[i] * b;
        }

        return result;
    }

    T dot(const Vector<T, Size>& b) const
    {
        T result;

        for (size_t i = 0; i < Size; i++)
        {
            result += v[i] * b.v[i];
        }

        return result;
    }
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

// A 4 by 4 matrix.
template <Scalar T> struct Matrix4
{
    T rows[4][4];

    // Constructs an identity matrix with the specified constant.
    Matrix4(T x = 1)
    {
        rows[0][0] = x;
        rows[0][1] = 0;
        rows[0][2] = 0;
        rows[0][3] = 0;

        rows[1][0] = 0;
        rows[1][1] = x;
        rows[1][2] = 0;
        rows[1][3] = 0;

        rows[2][0] = 0;
        rows[2][1] = 0;
        rows[2][2] = x;
        rows[2][3] = 0;

        rows[3][0] = 0;
        rows[3][1] = 0;
        rows[3][2] = 0;
        rows[3][3] = x;
    }

    // This function multiplies two matrices. Hopefully, in the future, I won't
    // have to tamper with this function, as it is very annoying to work with.
    Matrix4<T> operator*(const Matrix4<T>& b) const
    {
        Matrix4<T> result;

        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                result.rows[i][j] =
                    (rows[i][0] * b.rows[0][j]) + (rows[i][1] * b.rows[1][j]) +
                    (rows[i][2] * b.rows[2][j]) + (rows[i][3] * b.rows[3][j]);
            }
        }

        return result;
    }
};

// An alias for Matrices that uses floats.
using Matrix4f = Matrix4<float>;

// Returns the passed-in matrix with a specified translation transformation
// applied to it.
template <typename T>
Matrix4<T> translation(const Matrix4<T>& original, T x, T y, T z)
{
    Matrix4<T> transform;

    transform.rows[3][0] = x;
    transform.rows[3][1] = y;
    transform.rows[3][2] = z;

    return original * transform;
}

// Returns the passed-in matrix with a specified scale transformation applied
// to it.
template <typename T>
Matrix4<T> scale(const Matrix4<T>& original, T x, T y, T z)
{
    Matrix4<T> transform;

    transform.rows[0][0] = x;
    transform.rows[1][1] = y;
    transform.rows[2][2] = z;

    return original * transform;
}

} // namespace vulkan_scene