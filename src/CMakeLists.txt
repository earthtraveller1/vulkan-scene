target_sources(vulkan-scene PRIVATE
    buffers.cpp
    device.cpp
    main.cpp
    swap-chain.cpp

    buffers.hpp
    device.hpp
    pch.hpp
    swap-chain.hpp
    utils.hpp
    window.hpp
)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)

add_subdirectory(platform)