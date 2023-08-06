target_sources(
    vulkan-scene PRIVATE

    main.cpp
    device.cpp

    common.hpp
    device.hpp
)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)
