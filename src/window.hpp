#pragma once

namespace vulkan_scene
{
// Private window members. Implementation specific.
struct WindowImpl;

// A basic Windowing abstraction. It supports creating a Window and checking if
// it's still open. That's about it.
class Window
{
  public:
    Window(std::string_view title, uint16_t width, uint16_t height);

    // Creates a Vulkan surface object.
    VkSurfaceKHR create_surface(VkInstance instance) const;

    // Returns if the window is still open or not.
    bool is_open() const;

    // Obtains the window width and height.
    uint16_t get_width() const;
    uint16_t get_height() const;

    // Update the Window.
    void update();

    // Destroys the window.
    ~Window();

  private:
    // Window implementation.
    std::unique_ptr<WindowImpl> m_impl;
};
} // namespace vulkan_scene