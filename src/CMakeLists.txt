target_sources(vulkan-scene PRIVATE
    buffers.cpp
    device.cpp
    framebuffer-manager.cpp
    graphics-pipeline.cpp
    main.cpp
    swap-chain.cpp

    buffers.hpp
    device.hpp
    framebuffer-manager.hpp
    graphics-pipeline.hpp
    pch.hpp
    renderer.hpp
    swap-chain.hpp
    utils.hpp
    window.hpp
)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)

add_subdirectory(platform)