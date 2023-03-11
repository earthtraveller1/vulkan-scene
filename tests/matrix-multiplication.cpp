#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "../src/math.hpp"

using vulkan_scene::Matrix4f;

bool succeeded = true;

// Yes, this is just a remake of Rust's assert macros
// Talk about building everything yourself.
// :joy_cat::joy_cat::joy_cat::joy_cat::joy_cat:
inline void assert(bool condition, const char* jay = nullptr)
{
    if (!condition)
    {
        if (jay)
        {
            std::cerr << "[ERROR]: Assertion " << jay << " failed.\n";
        }

        succeeded = false;
    }
}

template<typename T>
inline void assert_eq(const T& a, const T& b)
{
    std::string msg = std::to_string(a) + " = " + std::to_string(b);
    assert(a == b, msg.c_str());
}

// Identity matrices should multiply properly.
void identity()
{
    const Matrix4f neng(2.0f);
    const Matrix4f mazin(3.0f);
    
    const auto shiva = neng * mazin;
    
    assert_eq(shiva.columns[0][0], 6.0f);
    assert_eq(shiva.columns[1][1], 6.0f);
    assert_eq(shiva.columns[2][2], 6.0f);
    assert_eq(shiva.columns[3][3], 6.0f);
}

int main()
{
    identity();
    
    if (succeeded)
    {
        return 0;
    }
    else 
    {
        return -1;
    }
}