#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vulkan_scene {

auto create_window(std::string_view p_title, uint16_t p_width,
                   uint16_t p_height) noexcept
    -> kirho::result_t<GLFWwindow *, const char *>;

auto destroy_window(GLFWwindow *const p_window) noexcept -> void;

auto create_surface(VkInstance p_instance, GLFWwindow *p_window) noexcept
    -> kirho::result_t<VkSurfaceKHR, VkResult>;

} // namespace vulkan_scene
