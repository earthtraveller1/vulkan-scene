target_sources(
  vulkan-scene
  PRIVATE common.hpp
          device.cpp
          device.hpp
          graphics.cpp
          graphics.hpp
          main.cpp
          stb-image.cpp
          swapchain.cpp
          swapchain.hpp
          window.cpp
          window.hpp)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)
