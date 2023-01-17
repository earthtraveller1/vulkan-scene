target_sources(vulkan-scene PRIVATE
    device.cpp
    main.cpp
    swap-chain.cpp

    device.hpp
    pch.hpp
    swap-chain.hpp
    window.hpp
)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)

add_subdirectory(platform)