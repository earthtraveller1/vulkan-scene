#include "window.hpp"
#include "common.hpp"

namespace vulkan_scene {

using kirho::result_t;

auto create_window(std::string_view p_title, uint16_t p_width,
                   uint16_t p_height) noexcept
    -> result_t<GLFWwindow *, const char *> {
  using result_t_t = result_t<GLFWwindow *, const char *>;

  if (!glfwInit()) {
    return result_t_t::error("[ERROR]: Failed to initialize GLFW.");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  const auto window =
      glfwCreateWindow(p_width, p_height, p_title.data(), nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    return result_t_t::error("Failed to create the GLFW window.");
  }

  return result_t_t::success(window);
}

auto destroy_window(GLFWwindow *const p_window) noexcept -> void {
  glfwDestroyWindow(p_window);
  glfwTerminate();
}

auto create_surface(VkInstance p_instance, GLFWwindow *p_window) noexcept
    -> result_t<VkSurfaceKHR, VkResult> {
  using result_t_t = result_t<VkSurfaceKHR, VkResult>;

  auto surface = static_cast<VkSurfaceKHR>(VK_NULL_HANDLE);
  const auto result_t =
      glfwCreateWindowSurface(p_instance, p_window, nullptr, &surface);
  if (result_t != VK_SUCCESS) {
    vulkan_scene::print_error(
        "Failed to create the window surface. Vulkan error ", result_t, ".");
    return result_t_t::error(result_t);
  }

  return result_t_t::success(surface);
}

} // namespace vulkan_scene
