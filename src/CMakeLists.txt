target_sources(
    vulkan-scene PRIVATE

    device.cpp
    main.cpp
    window.cpp

    common.hpp
    device.hpp
    window.hpp
)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)
