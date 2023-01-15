target_sources(vulkan-scene PRIVATE
    device.cpp
    main.cpp

    device.hpp
    window.hpp
)

add_subdirectory(platform)