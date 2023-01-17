#pragma once

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

    // The physical device and queue families
    VkPhysicalDevice m_physical_device;
    uint32_t m_graphics_queue_family;
    uint32_t m_present_queue_family;

    // The logical device and queues.
    VkDevice m_device;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;

    // Creates the instance.
    void create_instance(std::string_view application_name,
                         bool enable_validation);

    // Chooses a physical device.
    void choose_physical_device();
    
    // Creates the logical device and retrieves it's queues.
    void create_logical_device();
};
} // namespace vulkan_scene