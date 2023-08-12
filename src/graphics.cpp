#include <cstddef>
#include <cstring>

#include <fstream>

#include <vulkan/vulkan_core.h>

#include "common.hpp"
#include "device.hpp"

#include "graphics.hpp"

namespace
{

auto find_buffer_memory_type(
    VkPhysicalDevice p_device,
    uint32_t p_type_filter,
    VkMemoryPropertyFlags p_memory_properties
) -> kirho::result_t<uint32_t, kirho::empty>
{
    using result_t = kirho::result_t<uint32_t, kirho::empty>;

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

    return result_t::error(kirho::empty{});
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
) noexcept -> result_t<VkShaderModule, kirho::empty>
{
    using result_tt = result_t<VkShaderModule, kirho::empty>;

    std::ifstream file_stream{
        p_file_path.data(),
        std::ios::ate | std::ios::binary,
    };

    if (!file_stream)
    {
        vulkan_scene::print_error("Failed to open ", p_file_path, '.');
        return result_tt::error(kirho::empty{});
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
        return result_tt::error(kirho::empty{});
    }

    return result_tt::success(module);
}

auto create_pipeline_layout(VkDevice p_device) noexcept
    -> result_t<VkPipelineLayout, VkResult>
{
    using result_tt = result_t<VkPipelineLayout, VkResult>;

    const VkPipelineLayoutCreateInfo layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
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

    const std::array<VkVertexInputAttributeDescription, 1>
        vertex_attribute_descriptions{
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(vertex_t, position),
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
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

    const VkBufferCreateInfo staging_buffer_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = p_data_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    VkBuffer staging_buffer;
    auto result = vkCreateBuffer(
        p_device, &staging_buffer_info, nullptr, &staging_buffer
    );
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to create a Vulkan buffer. Vulkan error ", result, '.'
        );
        return result_t::error(result);
    }

    VkMemoryRequirements staging_memory_requirements;
    vkGetBufferMemoryRequirements(
        p_device, staging_buffer, &staging_memory_requirements
    );

    const auto staging_memory_type =
        find_buffer_memory_type(
            p_physical_device, staging_memory_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        )
            .unwrap();

    const VkMemoryAllocateInfo staging_memory_alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = staging_memory_requirements.size,
        .memoryTypeIndex = staging_memory_type,
    };

    VkDeviceMemory staging_buffer_memory;
    result = vkAllocateMemory(
        p_device, &staging_memory_alloc_info, nullptr, &staging_buffer_memory
    );
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to allocate memory for a staging buffer. Vulkan error ",
            result, '.'
        );
        return result_t::error(result);
    }

    vkBindBufferMemory(p_device, staging_buffer, staging_buffer_memory, 0);

    void* staging_buffer_pointer = nullptr;
    vkMapMemory(
        p_device, staging_buffer_memory, 0, p_data_size, 0,
        &staging_buffer_pointer
    );
    std::memcpy(staging_buffer_pointer, p_data, p_data_size);
    vkUnmapMemory(p_device, staging_buffer_memory);

    VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    switch (p_type)
    {
    case buffer_type_t::VERTEX:
        usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case buffer_type_t::INDEX:
        usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    };

    const VkBufferCreateInfo buffer_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = p_data_size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    VkBuffer buffer;
    result = vkCreateBuffer(p_device, &buffer_info, nullptr, &buffer);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to create a Vulkan buffer. Vulkan error ", result, '.'
        );
        return result_t::error(result);
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(p_device, buffer, &memory_requirements);

    const auto memory_type =
        find_buffer_memory_type(
            p_physical_device, memory_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        )
            .unwrap();

    const VkMemoryAllocateInfo memory_alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type,
    };

    VkDeviceMemory buffer_memory;
    result =
        vkAllocateMemory(p_device, &memory_alloc_info, nullptr, &buffer_memory);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to allocate memory for a buffer. Vulkan error ", result, '.'
        );
        return result_t::error(result);
    }

    vkBindBufferMemory(p_device, buffer, buffer_memory, 0);

    const auto command_buffer_result =
        create_command_buffer(p_device, p_command_pool);
    if (command_buffer_result.is_error(result))
    {
        return result_t::error(result);
    }

    const auto command_buffer = command_buffer_result.unwrap();

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    result = vkBeginCommandBuffer(command_buffer, &begin_info);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to begin a command buffer. Vulkan error ", result, "."
        );
        return result_t::error(result);
    }

    const VkBufferCopy copy_region{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = p_data_size,
    };

    vkCmdCopyBuffer(command_buffer, staging_buffer, buffer, 1, &copy_region);

    result = vkEndCommandBuffer(command_buffer);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to end a command buffer. Vulkan error ", result, "."
        );
        return result_t::error(result);
    }

    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    result = vkQueueSubmit(p_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to submit a command buffer. Vulkan error ", result, "."
        );
        return result_t::error(result);
    }

    result = vkQueueWaitIdle(p_graphics_queue);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to wait for the queue to complete its operations. Vulkan "
            "error ",
            result, "."
        );
        return result_t::error(result);
    }

    vkDestroyBuffer(p_device, staging_buffer, nullptr);
    vkFreeMemory(p_device, staging_buffer_memory, nullptr);

    return result_t::success(buffer_t{
        .buffer = buffer,
        .memory = buffer_memory,
        .type = p_type,
    });
}

auto destroy_buffer(VkDevice p_device, const buffer_t& p_buffer) -> void
{
    vkDestroyBuffer(p_device, p_buffer.buffer, nullptr);
    vkFreeMemory(p_device, p_buffer.memory, nullptr);
}

} // namespace vulkan_scene
