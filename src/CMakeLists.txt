target_sources(
  vulkan-scene
  PRIVATE device.cpp
          graphics.cpp
          main.cpp
          window.cpp
          common.hpp
          device.hpp
          swapchain.cpp
          swapchain.hpp
          graphics.hpp
          window.hpp)

target_precompile_headers(vulkan-scene PRIVATE pch.hpp)
