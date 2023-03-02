#include <cstdint>
#include <cstdlib>

#include "../src/math.hpp"

using vulkan_scene::Matrix4f;

// Yes, this is just a remake of Rust's assert macros
// Talk about building everything yourself.
// :joy_cat::joy_cat::joy_cat::joy_cat::joy_cat:
inline void assert(bool condition)
{
    if (!condition)
    {
        std::exit(-1);
    }
}

template<typename T>
inline void assert_eq(const T& a, const T& b)
{
    assert(a == b);
}

// Identity matrices should multiply properly.
void identity()
{
    const Matrix4f neng(2.0f);
    const Matrix4f mazin(3.0f);
    
    const auto shiva = neng * mazin;
    
    assert_eq(shiva.rows[0][0], 6.0f);
    assert_eq(shiva.rows[1][1], 6.0f);
    assert_eq(shiva.rows[2][2], 6.0f);
    assert_eq(shiva.rows[3][3], 6.0f);
}

int main()
{
    identity();
    return 0;
}