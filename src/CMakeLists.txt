target_sources(
    vulkan-scene PRIVATE

    main.cpp
)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)
