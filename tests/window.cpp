#include <cassert>

#include <window.hpp>

auto main() -> int
{
    // This should succeed, as it's a valid window.
    vulkan_scene::create_window("Hello!", 800, 600).unwrap();

    // This should fail.
    const char* foo;
    assert(vulkan_scene::create_window("Hello!", 0, 0).is_error(foo));
    (void)foo;

    return 0;
}
