#pragma once

#include <string_view>

#include <vulkan/vulkan.h>

namespace vulkan_scene
{
    // A basic wrapper around a bunch of Vulkan objects that represents the Vulkan device.
    // It includes Vulkan instance, the surface, the logical device, queues, and related stuff.
    class Device
    {
    public:
        // Creates a device with the option of enabling validation and specifying the application
        // name.
        Device(std::string_view application_name, bool enable_validation);

        // Destructor
        ~Device();

    private:
        // The handle to the Vulkan instance.
        VkInstance m_instance;

        // Creates the instance.
        void create_instance(std::string_view application_name, bool enable_validation);
    };
}