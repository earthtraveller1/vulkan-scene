#pragma once

#include <string_view>

#include <vulkan/vulkan.h>

namespace vulkan_scene
{
class Window;

// A basic wrapper around a bunch of Vulkan objects that represents the Vulkan
// device. It includes Vulkan instance, the surface, the logical device, queues,
// and related stuff.
class Device
{
  public:
    // Creates a device with the option of enabling validation and specifying
    // the application name.
    Device(std::string_view application_name, bool enable_validation,
           const Window& window);

    // Can't copy.
    Device(const Device& src) = delete;
    Device& operator=(const Device& rhs) = delete;

    // Destructor
    ~Device();

  private:
    // The handle to the Vulkan instance.
    VkInstance m_instance;

    // The handle to the Vulkan surface.
    VkSurfaceKHR m_surface;

    // Creates the instance.
    void create_instance(std::string_view application_name,
                         bool enable_validation);
};
} // namespace vulkan_scene