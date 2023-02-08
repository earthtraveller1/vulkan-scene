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

    // Obtains the raw handle to the Vulkan device. Is used internally.
    VkDevice get_raw_handle() const { return m_device; }

    // Obtains the raw handle to the Vulkan physical device. Is used internally.
    VkPhysicalDevice get_raw_physical_handle() const
    {
        return m_physical_device;
    }

    VkQueue get_graphics_queue() const { return m_graphics_queue; }
    
    VkQueue get_present_queue() const { return m_present_queue; }

    uint32_t get_graphics_queue_family() const
    {
        return m_graphics_queue_family;
    }

    uint32_t get_present_queue_family() const { return m_present_queue_family; }

    VkSurfaceKHR get_raw_surface_handle() const { return m_surface; }

    // Probably shouldn't be used in client code.
    VkSemaphore create_semaphore() const;

    // Probably shouldn't be used in client code.
    VkFence create_fence(bool signaled) const;

    // Allocates a primary command buffer. Used internally.
    // The argument tells you whether it's a one use buffer or not.
    VkCommandBuffer allocate_primary_cmd_buffer() const;
    
    // Waits for the device to complete all remaining operations.
    inline void wait_idle() const { vkDeviceWaitIdle(m_device); }

    // Deallocates a command buffer.
    void free_command_buffer(VkCommandBuffer command_buffer) const
    {
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
    }

    inline void destroy_semaphore(VkSemaphore semaphore) const
    {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

    inline void destroy_fence(VkFence fence) const
    {
        vkDestroyFence(m_device, fence, nullptr);
    }

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

    // The command pool.
    VkCommandPool m_command_pool;

    // Creates the instance.
    void create_instance(std::string_view application_name,
                         bool enable_validation);

    // Chooses a physical device.
    void choose_physical_device();

    // Creates the logical device and retrieves it's queues.
    void create_logical_device();

    // Creates the command poool.
    void create_command_pool();
};
} // namespace vulkan_scene