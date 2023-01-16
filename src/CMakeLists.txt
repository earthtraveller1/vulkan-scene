target_sources(vulkan-scene PRIVATE
    device.cpp
    main.cpp

    device.hpp
    pch.hpp
    window.hpp
)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)

add_subdirectory(platform)