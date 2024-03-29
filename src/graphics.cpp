#include <cstddef>
#include <cstring>

#include <exception>
#include <fstream>

#include <stb_image.h>
#include <vulkan/vulkan_core.h>

#include "common.hpp"
#include "device.hpp"

#include "graphics.hpp"

namespace
{

using vulkan_scene::buffer_t;
using vulkan_scene::print_error;

class temporary_command_buffer_t
{
  public:
    temporary_command_buffer_t(
        VkDevice device, VkQueue queue, VkCommandPool pool
    )
        : m_device(device), m_queue(queue), m_pool(pool),
          m_buffer(vulkan_scene::create_command_buffer(device, pool).unwrap())
    {
        const VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
        };

        const auto result = vkBeginCommandBuffer(m_buffer, &begin_info);
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to begin a single use command buffer. Vulkan error ",
                result
            );
        }
    }

    temporary_command_buffer_t(const temporary_command_buffer_t&) = delete;
    temporary_command_buffer_t& operator=(const temporary_command_buffer_t&) =
        delete;

    operator VkCommandBuffer()
    {
        return m_buffer;
    }

    ~temporary_command_buffer_t()
    {
        auto result = vkEndCommandBuffer(m_buffer);
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to end a single use command buffer. Vulkan error ",
                result
            );
            return;
        }

        const VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_buffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };

        result = vkQueueSubmit(m_queue, 1, &submit_info, VK_NULL_HANDLE);
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to submit a single use command buffer. Vulkan error ",
                result
            );
            return;
        }

        vkQueueWaitIdle(m_queue);

        vkFreeCommandBuffers(m_device, m_pool, 1, &m_buffer);
    }

  private:
    VkDevice m_device;
    VkQueue m_queue;
    VkCommandPool m_pool;
    VkCommandBuffer m_buffer;
};

auto find_buffer_memory_type(
    VkPhysicalDevice p_device,
    uint32_t p_type_filter,
    VkMemoryPropertyFlags p_memory_properties
) -> kirho::result_t<uint32_t, kirho::empty_t>
{
    using result_t = kirho::result_t<uint32_t, kirho::empty_t>;

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(p_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        const auto follows_filter = (p_type_filter & (1 << i)) != 0;
        const auto has_properties =
            (memory_properties.memoryTypes[i].propertyFlags &
             p_memory_properties) == p_memory_properties;

        if (follows_filter && has_properties)
        {
            return result_t::success(i);
        }
    }

    return result_t::error(kirho::empty_t{});
}

auto create_vulkan_buffer(
    VkPhysicalDevice p_physical_device,
    VkDevice p_device,
    VkDeviceSize p_buffer_size,
    VkBufferUsageFlags p_usage,
    VkMemoryPropertyFlags p_memory_properties
) noexcept -> kirho::result_t<buffer_t, VkResult>
{
    using result_t = kirho::result_t<buffer_t, VkResult>;

    buffer_t buffer{};

    const VkBufferCreateInfo buffer_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = p_buffer_size,
        .usage = p_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    const auto result =
        vkCreateBuffer(p_device, &buffer_info, nullptr, &buffer.buffer);
    if (result != VK_SUCCESS)
    {
        print_error("Failed to create a buffer. Vulkan error ", result, '.');
        return result_t::error(result);
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(
        p_device, buffer.buffer, &memory_requirements
    );

    const auto memory_type_result = find_buffer_memory_type(
        p_physical_device, memory_requirements.memoryTypeBits,
        p_memory_properties
    );
    {
        kirho::empty_t empty{};
        if (memory_type_result.is_error(empty))
        {
            return result_t::error(VK_ERROR_UNKNOWN);
        }
    }
    const auto memory_type = memory_type_result.unwrap();

    const VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type,
    };

    const auto result2 =
        vkAllocateMemory(p_device, &alloc_info, nullptr, &buffer.memory);
    if (result2 != VK_SUCCESS)
    {
        print_error(
            "Failed to allocate memory for a buffer. Vulkan error ", result2,
            '.'
        );
        return result_t::error(result2);
    }

    vkBindBufferMemory(p_device, buffer.buffer, buffer.memory, 0);

    return result_t::success(buffer);
}

auto transition_image_layout(
    VkDevice p_device,
    VkQueue p_queue,
    VkCommandPool p_pool,
    VkImage p_image,
    VkFormat p_format,
    VkImageLayout p_old_layout,
    VkImageLayout p_new_layout
) -> kirho::result_t<kirho::empty_t, std::string_view>
{
    using result_t = kirho::result_t<kirho::empty_t, std::string_view>;

    temporary_command_buffer_t command_buffer(p_device, p_queue, p_pool);

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = p_old_layout,
        .newLayout = p_new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = p_image,
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VkPipelineStageFlags sourceStage, destinationStage;

    if (p_old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        p_new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (p_old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        return result_t::error("Unsupported layout transition");
    }

    vkCmdPipelineBarrier(
        command_buffer, sourceStage, destinationStage, 0, 0, nullptr, 0,
        nullptr, 1, &barrier
    );

    return result_t::success();
}

} // namespace

namespace vulkan_scene
{

using kirho::result_t;

auto create_render_pass(VkDevice p_device, VkFormat p_swapchain_format) noexcept
    -> result_t<VkRenderPass, VkResult>
{
    const VkAttachmentDescription attachment{
        .flags = 0,
        .format = p_swapchain_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkAttachmentReference attachment_ref{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_ref,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    const VkSubpassDependency subpass_dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0,
    };

    const VkRenderPassCreateInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpass_dependency, // TODO
    };

    using result_tt = result_t<VkRenderPass, VkResult>;

    VkRenderPass render_pass;
    const auto result =
        vkCreateRenderPass(p_device, &render_pass_info, nullptr, &render_pass);
    if (result != VK_SUCCESS)
    {
        vulkan_scene::print_error(
            "Failed to create the render pass. Vulkan error ", result
        );
        return result_tt::error(result);
    }

    return result_tt::success(render_pass);
}

auto create_shader_module(
    VkDevice p_device, std::string_view p_file_path
) noexcept -> result_t<VkShaderModule, kirho::empty_t>
{
    using result_tt = result_t<VkShaderModule, kirho::empty_t>;

    std::ifstream file_stream{
        p_file_path.data(),
        std::ios::ate | std::ios::binary,
    };

    if (!file_stream)
    {
        vulkan_scene::print_error("Failed to open ", p_file_path, '.');
        return result_tt::error(kirho::empty_t{});
    }

    const auto file_size = static_cast<size_t>(file_stream.tellg());
    std::vector<char> file_content(file_size);

    file_stream.seekg(0);
    file_stream.read(file_content.data(), file_size);

    file_stream.close();

    const VkShaderModuleCreateInfo module_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = file_content.size(),
        .pCode = reinterpret_cast<const uint32_t*>(file_content.data()),
    };

    VkShaderModule module;
    const auto result =
        vkCreateShaderModule(p_device, &module_info, nullptr, &module);
    if (result != VK_SUCCESS)
    {
        vulkan_scene::print_error(
            "Failed to create a shader module from ", p_file_path,
            ". Vulkan error ", result, "."
        );
        return result_tt::error(kirho::empty_t{});
    }

    return result_tt::success(module);
}

auto create_pipeline_layout(
    VkDevice p_device,
    std::span<const VkDescriptorSetLayout> p_descriptor_set_layouts,
    std::span<const VkPushConstantRange> p_push_constant_ranges
) noexcept -> result_t<VkPipelineLayout, VkResult>
{
    using result_tt = result_t<VkPipelineLayout, VkResult>;

    const VkPipelineLayoutCreateInfo layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount =
            static_cast<uint32_t>(p_descriptor_set_layouts.size()),
        .pSetLayouts = p_descriptor_set_layouts.data(),
        .pushConstantRangeCount =
            static_cast<uint32_t>(p_push_constant_ranges.size()),
        .pPushConstantRanges = p_push_constant_ranges.data(),
    };

    VkPipelineLayout layout;
    const auto result =
        vkCreatePipelineLayout(p_device, &layout_info, nullptr, &layout);
    if (result != VK_SUCCESS)
    {
        vulkan_scene::print_error(
            "Failed to create the pipeline layout. Vulkan error ", result, '.'
        );
        return result_tt::error(result);
    }

    return result_tt::success(layout);
}

auto create_graphics_pipeline(
    VkDevice p_device,
    VkRenderPass p_render_pass,
    VkPipelineLayout p_layout,
    VkShaderModule p_vertex_shader,
    VkShaderModule p_fragment_shader
) noexcept -> result_t<VkPipeline, VkResult>
{
    using result_tt = result_t<VkPipeline, VkResult>;

    const VkPipelineShaderStageCreateInfo vertex_shader_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = p_vertex_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };

    const VkPipelineShaderStageCreateInfo fragment_shader_stage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = p_fragment_shader,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };

    const std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{
        vertex_shader_stage, fragment_shader_stage};

    const std::array<VkDynamicState, 2> dynamic_states{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    const VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = 2, // Set the number of dynamic states
        .pDynamicStates = dynamic_states.data(),
    };

    const VkVertexInputBindingDescription vertex_binding_description{
        .binding = 0,
        .stride = sizeof(vertex_t),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    const std::array<VkVertexInputAttributeDescription, 3>
        vertex_attribute_descriptions{
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = static_cast<uint32_t>(offsetof(vertex_t, position)),
            },
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = static_cast<uint32_t>(offsetof(vertex_t, uv)),
            },
            VkVertexInputAttributeDescription{
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = static_cast<uint32_t>(offsetof(vertex_t, normal)),
            },
        };

    const VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_binding_description,
        .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(vertex_attribute_descriptions.size()),
        .pVertexAttributeDescriptions = vertex_attribute_descriptions.data(),
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    const VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    const VkPipelineRasterizationStateCreateInfo rasterization_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f};

    const VkPipelineMultisampleStateCreateInfo multisample_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    const VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    const VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    const VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = &dynamic_state,
        .layout = p_layout,
        .renderPass = p_render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VkPipeline pipeline;
    const auto result = vkCreateGraphicsPipelines(
        p_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline
    );
    if (result != VK_SUCCESS)
    {
        vulkan_scene::print_error(
            "Failed to create the graphics pipeline. Vulkan error ", result, '.'
        );
        return result_tt::error(result);
    }

    return result_tt::success(pipeline);
}

auto create_buffer(
    VkPhysicalDevice p_physical_device,
    VkDevice p_device,
    VkQueue p_graphics_queue,
    VkCommandPool p_command_pool,
    buffer_type_t p_type,
    const void* p_data,
    VkDeviceSize p_data_size
) noexcept -> kirho::result_t<buffer_t, VkResult>
{
    using result_t = kirho::result_t<buffer_t, VkResult>;

    const auto staging_buffer_result = create_vulkan_buffer(
        p_physical_device, p_device, p_data_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    {
        VkResult error;
        if (staging_buffer_result.is_error(error))
        {
            return result_t::error(error);
        }
    }
    const auto staging_buffer = staging_buffer_result.unwrap();

    void* staging_buffer_pointer = nullptr;
    vkMapMemory(
        p_device, staging_buffer.memory, 0, p_data_size, 0,
        &staging_buffer_pointer
    );
    std::memcpy(staging_buffer_pointer, p_data, p_data_size);
    vkUnmapMemory(p_device, staging_buffer.memory);

    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    switch (p_type)
    {
    case buffer_type_t::VERTEX:
        usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case buffer_type_t::INDEX:
        usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    default:
        print_error("Invalid buffer type.");
        return result_t::error(VK_ERROR_UNKNOWN);
    };

    const auto buffer_result = create_vulkan_buffer(
        p_physical_device, p_device, p_data_size, usage_flags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    {
        VkResult error;
        if (buffer_result.is_error(error))
        {
            return result_t::error(error);
        }
    }
    auto buffer = buffer_result.unwrap();
    buffer.type = p_type;

    {
        temporary_command_buffer_t command_buffer(
            p_device, p_graphics_queue, p_command_pool
        );

        const VkBufferCopy copy_region{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = p_data_size,
        };

        vkCmdCopyBuffer(
            command_buffer, staging_buffer.buffer, buffer.buffer, 1,
            &copy_region
        );
    }

    destroy_buffer(p_device, staging_buffer);

    return result_t::success(buffer);
}

auto create_uniform_buffer(
    VkPhysicalDevice p_physical_device,
    VkDevice p_device,
    void* p_data,
    VkDeviceSize p_data_size
) noexcept -> kirho::result_t<buffer_t, VkResult>
{
    using result_t = kirho::result_t<buffer_t, VkResult>;

    const auto buffer_result = create_vulkan_buffer(
        p_physical_device, p_device, p_data_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    {
        VkResult error;
        if (buffer_result.is_error(error))
        {
            return result_t::error(error);
        }
    }

    auto buffer = buffer_result.unwrap();
    buffer.type = buffer_type_t::UNIFORM;

    return result_t::success(buffer);
}

auto create_image(
    VkPhysicalDevice p_physical_device,
    VkDevice p_device,
    VkQueue p_queue,
    VkCommandPool p_command_pool,
    std::string_view p_file_path
) -> kirho::result_t<image_t, VkResult>
{
    using result_t = kirho::result_t<image_t, VkResult>;

    constexpr auto fixed_channels = 4;

    int width, height, channels;
    const auto image_data = stbi_load(
        p_file_path.data(), &width, &height, &channels, fixed_channels
    );

    if (image_data == nullptr)
    {
        print_error("Failed to load ", p_file_path, '.');
        return result_t::error(VK_ERROR_UNKNOWN);
    }

    const VkDeviceSize buffer_size =
        width * height * fixed_channels * sizeof(*image_data);

    const auto staging_buffer = create_vulkan_buffer(
                                    p_physical_device, p_device, buffer_size,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    )
                                    .unwrap();

    void* staging_buffer_pointer;
    vkMapMemory(
        p_device, staging_buffer.memory, 0, buffer_size, 0,
        &staging_buffer_pointer
    );
    std::memcpy(staging_buffer_pointer, image_data, buffer_size);
    vkUnmapMemory(p_device, staging_buffer.memory);

    stbi_image_free(image_data);

    const VkImageCreateInfo image_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent =
            VkExtent3D{
                .width = static_cast<uint32_t>(width),
                .height = static_cast<uint32_t>(height),
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image;
    auto result = vkCreateImage(p_device, &image_info, nullptr, &image);
    if (result != VK_SUCCESS)
    {
        print_error("Failed to create an image. Vulkan error ", result);
        return result_t::error(result);
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(p_device, image, &memory_requirements);

    const VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex =
            find_buffer_memory_type(
                p_physical_device, memory_requirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            )
                .unwrap(),
    };

    VkDeviceMemory memory;
    result = vkAllocateMemory(p_device, &alloc_info, nullptr, &memory);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to allocate memory for an image. Vulkan error ", result
        );
        return result_t::error(result);
    }

    vkBindImageMemory(p_device, image, memory, 0);

    std::string_view error;
    if (transition_image_layout(
            p_device, p_queue, p_command_pool, image, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        )
            .is_error(error))
    {
        print_error(error);
        return result_t::error(VK_ERROR_UNKNOWN);
    }

    {
        temporary_command_buffer_t command_buffer{
            p_device, p_queue, p_command_pool};

        const VkBufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageOffset =
                VkOffset3D{
                    .x = 0,
                    .y = 0,
                    .z = 0,
                },
            .imageExtent =
                VkExtent3D{
                    .width = static_cast<uint32_t>(width),
                    .height = static_cast<uint32_t>(height),
                    .depth = 1,
                },
        };

        vkCmdCopyBufferToImage(
            command_buffer, staging_buffer.buffer, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region
        );
    }

    destroy_buffer(p_device, staging_buffer);

    if (transition_image_layout(
            p_device, p_queue, p_command_pool, image, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        )
            .is_error(error))
    {
        print_error(error);
        return result_t::error(VK_ERROR_UNKNOWN);
    }

    const auto image_view_result = create_image_view(p_device, image);
    if (image_view_result.is_error(result))
    {
        return result_t::error(result);
    }

    const auto image_view = image_view_result.unwrap();

    return result_t::success(image_t{
        .image = image,
        .memory = memory,
        .view = image_view,
    });
}

auto create_image_view(VkDevice p_device, VkImage p_image, VkFormat p_format)
    -> kirho::result_t<VkImageView, VkResult>
{
    using result_t = kirho::result_t<VkImageView, VkResult>;

    const VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = p_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = p_format,
        .components =
            VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VkImageView view;
    const auto result = vkCreateImageView(p_device, &view_info, nullptr, &view);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to create an image view. Vulkan error ", result, '.'
        );
        return result_t::error(result);
    }

    return result_t::success(view);
}

auto create_sampler(
    VkDevice p_device,
    VkFilter p_min_filter,
    VkFilter p_mag_filter,
    bool enable_anisothropy
) -> kirho::result_t<VkSampler, VkResult>
{
    using result_t = kirho::result_t<VkSampler, VkResult>;

    const VkSamplerCreateInfo sampler_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = p_mag_filter,
        .minFilter = p_min_filter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = enable_anisothropy ? VK_TRUE : VK_FALSE,
        .maxAnisotropy = 0.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VkSampler sampler;
    const auto result =
        vkCreateSampler(p_device, &sampler_info, nullptr, &sampler);
    if (result != VK_SUCCESS)
    {
        print_error("Failed to create a sampler. Vulkan error ", result, '.');
        return result_t::error(result);
    }

    return result_t::success(sampler);
}

auto destroy_buffer(VkDevice p_device, const buffer_t& p_buffer) -> void
{
    vkDestroyBuffer(p_device, p_buffer.buffer, nullptr);
    vkFreeMemory(p_device, p_buffer.memory, nullptr);
}

auto destroy_image(VkDevice p_device, const image_t& p_image) -> void
{
    vkDestroyImage(p_device, p_image.image, nullptr);
    vkFreeMemory(p_device, p_image.memory, nullptr);
    vkDestroyImageView(p_device, p_image.view, nullptr);
}

} // namespace vulkan_scene
