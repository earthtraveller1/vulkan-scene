#include "device.hpp"
#include <algorithm>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>

#include "common.hpp"

namespace {

using kirho::result_t;

constexpr uint16_t WINDOW_WIDTH = 1280;
constexpr uint16_t WINDOW_HEIGHT = 720;

// May throw an std::runtime_error.
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

struct physical_device {
  VkPhysicalDevice device;
  uint32_t graphics_family;
  uint32_t present_family;
};

struct swapchain_t {
  VkSwapchainKHR swapchain;
  std::vector<VkImage> images;
  VkFormat format;
  VkExtent2D extent;
};

auto create_swapchain(VkDevice p_device, VkPhysicalDevice p_physical_device,
                      uint32_t p_graphics_family, uint32_t p_present_family,
                      GLFWwindow *p_window, VkSurfaceKHR p_surface) noexcept
    -> result_t<swapchain_t, VkResult> {
  VkSurfaceCapabilitiesKHR surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_physical_device, p_surface,
                                            &surface_capabilities);

  uint32_t surface_format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface,
                                       &surface_format_count, nullptr);

  std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface,
                                       &surface_format_count,
                                       surface_formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface,
                                            &present_mode_count, nullptr);

  std::vector<VkPresentModeKHR> present_modes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      p_physical_device, p_surface, &present_mode_count, present_modes.data());

  VkSurfaceFormatKHR surface_format = surface_formats.front();

  for (const auto &format : surface_formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      surface_format = format;
      break;
    }
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

  for (auto mode : present_modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = mode;
      break;
    }
  }

  int framebuffer_width, framebuffer_height;
  glfwGetFramebufferSize(p_window, &framebuffer_width, &framebuffer_height);

  const VkExtent2D swap_extent =
      surface_capabilities.currentExtent.width !=
              std::numeric_limits<uint32_t>::max()
          ? surface_capabilities.currentExtent
          : VkExtent2D{
                std::clamp(static_cast<uint32_t>(framebuffer_width),
                           surface_capabilities.minImageExtent.width,
                           surface_capabilities.maxImageExtent.width),
                std::clamp(static_cast<uint32_t>(framebuffer_height),
                           surface_capabilities.minImageExtent.height,
                           surface_capabilities.maxImageExtent.height),
            };

  uint32_t image_count = surface_capabilities.minImageCount + 1;
  const auto has_max_image_count = surface_capabilities.maxImageCount > 0;
  if (has_max_image_count && image_count > surface_capabilities.maxImageCount) {
    image_count = surface_capabilities.maxImageCount;
  }

  const auto queue_families_same = p_graphics_family == p_present_family;

  std::array<uint32_t, 2> queue_families{p_graphics_family, p_present_family};

  const VkSwapchainCreateInfoKHR swapchain_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .surface = p_surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = swap_extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = queue_families_same ? VK_SHARING_MODE_EXCLUSIVE
                                              : VK_SHARING_MODE_CONCURRENT,
      .queueFamilyIndexCount =
          queue_families_same ? 0
                              : static_cast<uint32_t>(queue_families.size()),
      .pQueueFamilyIndices =
          queue_families_same ? nullptr : queue_families.data(),
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE,
  };

  VkSwapchainKHR swapchain;

  using result_t_t = result_t<swapchain_t, VkResult>;

  const auto result_t =
      vkCreateSwapchainKHR(p_device, &swapchain_info, nullptr, &swapchain);
  if (result_t != VK_SUCCESS) {
    vulkan_scene::print_error("Failed to create the swapchain. Vulkan error ",
                              result_t);
    return result_t_t::error(result_t);
  }

  vkGetSwapchainImagesKHR(p_device, swapchain, &image_count, nullptr);
  std::vector<VkImage> images(image_count);
  vkGetSwapchainImagesKHR(p_device, swapchain, &image_count, images.data());

  return result_t_t::success(
      swapchain_t{swapchain, images, surface_format.format, swap_extent});
}

auto create_image_views(VkDevice p_device, const std::vector<VkImage> &p_images,
                        VkFormat p_format)
    -> result_t<std::vector<VkImageView>, VkResult> {
  using result_t_t = result_t<std::vector<VkImageView>, VkResult>;

  std::vector<VkImageView> image_views(p_images.size());

  VkResult latest_failure = VK_SUCCESS;

  std::transform(
      p_images.cbegin(), p_images.cend(), image_views.begin(),
      [p_device, p_format, &latest_failure](VkImage p_image) -> VkImageView {
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
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        VkImageView image_view;
        const auto result =
            vkCreateImageView(p_device, &view_info, nullptr, &image_view);
        if (result != VK_SUCCESS) {
          latest_failure = result;
          vulkan_scene::print_error(
              "Failed to create an image view. Vulkan error ", result, '.');
        }

        return image_view;
      });

  if (latest_failure != VK_SUCCESS) {
    return result_t_t::error(latest_failure);
  }

  return result_t_t::success(image_views);
}

auto create_render_pass(VkDevice p_device, VkFormat p_swapchain_format)
    -> result_t<VkRenderPass, VkResult> {
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
  if (result != VK_SUCCESS) {
    vulkan_scene::print_error("Failed to create the render pass. Vulkan error ",
                              result);
    return result_tt::error(result);
  }

  return result_tt::success(render_pass);
}

auto create_framebuffers(VkDevice p_device,
                         const std::vector<VkImageView> &p_image_views,
                         const VkExtent2D &p_swapchain_extent,
                         VkRenderPass p_render_pass)
    -> result_t<std::vector<VkFramebuffer>, VkResult> {
  std::vector<VkFramebuffer> framebuffers;

  using result_tt = result_t<std::vector<VkFramebuffer>, VkResult>;

  VkResult latest_failure = VK_SUCCESS;

  std::ranges::transform(
      p_image_views, std::back_inserter(framebuffers),
      [p_device, p_render_pass, &p_swapchain_extent,
       &latest_failure](VkImageView p_image_view) -> VkFramebuffer {
        const VkFramebufferCreateInfo framebuffer_info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = p_render_pass,
            .attachmentCount = 1,
            .pAttachments = &p_image_view,
            .width = p_swapchain_extent.width,
            .height = p_swapchain_extent.height,
            .layers = 1,
        };

        VkFramebuffer framebuffer;
        const auto result = vkCreateFramebuffer(p_device, &framebuffer_info,
                                                nullptr, &framebuffer);
        if (result != VK_SUCCESS) {
          vulkan_scene::print_error(
              "Failed to create a Vulkan framebuffer. Vulkan error ", result,
              '.');
          latest_failure = result;
        }

        return framebuffer;
      });

  if (latest_failure != VK_SUCCESS) {
    return result_tt::error(latest_failure);
  }

  return result_tt::success(framebuffers);
}

} // namespace

auto main() noexcept -> int {
  const auto enable_validation = true;

  const auto window =
      create_window("Vulkan Scene", WINDOW_WIDTH, WINDOW_HEIGHT).unwrap();
  defer(window, destroy_window(window));

  const auto instance =
      vulkan_scene::create_vulkan_instance(enable_validation).unwrap();
  defer(instance, vkDestroyInstance(instance, nullptr));

  const auto debug_messenger =
      enable_validation
          ? vulkan_scene::create_debug_messenger(instance).unwrap()
          : VK_NULL_HANDLE;
  defer(debug_messenger,
        enable_validation
            ? vulkan_scene::destroy_debug_messenger(instance, debug_messenger)
            : (void)0);

  const auto surface = create_surface(instance, window).unwrap();
  defer(surface, vkDestroySurfaceKHR(instance, surface, nullptr));

  const auto [physical_device, graphics_queue_family, present_queue_family] =
      vulkan_scene::choose_physical_device(instance, surface).unwrap();

  const auto logical_device =
      vulkan_scene::create_logical_device(
          physical_device, graphics_queue_family, present_queue_family)
          .unwrap();
  defer(logical_device, vkDestroyDevice(logical_device, nullptr));

  const auto swapchain =
      create_swapchain(logical_device, physical_device, graphics_queue_family,
                       present_queue_family, window, surface)
          .unwrap();
  defer(swapchain,
        vkDestroySwapchainKHR(logical_device, swapchain.swapchain, nullptr));

  const auto swapchain_image_views =
      create_image_views(logical_device, swapchain.images, swapchain.format)
          .unwrap();
  defer(swapchain_image_views,
        std::for_each(swapchain_image_views.cbegin(),
                      swapchain_image_views.cend(),
                      [logical_device](VkImageView p_view) {
                        vkDestroyImageView(logical_device, p_view, nullptr);
                      }));

  const auto render_pass =
      create_render_pass(logical_device, swapchain.format).unwrap();
  defer(render_pass, vkDestroyRenderPass(logical_device, render_pass, nullptr));

  const auto framebuffers =
      create_framebuffers(logical_device, swapchain_image_views,
                          swapchain.extent, render_pass)
          .unwrap();
  defer(framebuffers,
        std::ranges::for_each(framebuffers, [logical_device](auto buffer) {
          vkDestroyFramebuffer(logical_device, buffer, nullptr);
        }));

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  return 0;
}
