#pragma once

#define vulkan_scene_VK_CHECK(res, msg)                                        \
    if (res != VK_SUCCESS)                                                     \
    {                                                                          \
        throw std::runtime_error(                                              \
            std::string("Failed to " msg ". Vulkan error ") +                  \
            std::to_string(res) + '.');                                        \
    }
