#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core.h"
#include "Descriptor.h"
#include "Helper.h"
#include "Pipeline.h"

#define VULKAN_VERSION VK_API_VERSION_1_3

#define MAX_DESCRIPTOR_SETS 1024

namespace Ash {

class VulkanAPI {
public:
  VulkanAPI();
  ~VulkanAPI();

  void init(const std::vector<Pipeline> &pipelines);
  void render();
  void cleanup();

  void setClearColor(const glm::vec4 &color);

  IndexedVertexBuffer
  createIndexedVertexArray(const std::vector<Vertex> &verts,
                           const std::vector<uint32_t> &indices);
  void createRenderableDescriptorSets(std::vector<vk::DescriptorSet> &sets,
                                      const std::vector<UniformBuffer> &ubo,
                                      Material &material);
  void createUniformBuffers(std::vector<UniformBuffer> &ubos,
                            vk::DeviceSize bufferSize);
  void createTextureImage(const std::string &path, Texture &texture);
  void createTextureImageView(Texture &texture);

  DescriptorLayoutCache descriptorLayoutCache;
  DescriptorAllocator descriptorAllocator;

private:
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentsFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentsFamily.has_value();
    }
  };

  struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
  };

  vk::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
  bool checkValidationSupport();
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createAllocator();
  void createSwapchain();
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayouts();
  void createPipelineCache();
  void createGlobalDescriptorSets();
  void createGraphicsPipelines(const std::vector<Pipeline> &pipelines);
  void createFramebuffers();
  void createDescriptorAllocator();
  void createCommandPools();
  void createCommandBuffers();
  void recordCommandBuffers();
  void createSyncObjects();
  void cleanupSwapchain();
  void recreateSwapchain();
  void updateCommandBuffers();
  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
                                 vk::ImageTiling tiling,
                                 vk::FormatFeatureFlags features);
  bool hasStencilComponent(vk::Format format);
  vk::Format findDepthFormat();
  void createDepthResources();
  void createBuffer(vk::DeviceSize size, VmaMemoryUsage memUsage,
                    vk::BufferUsageFlags usage, vk::Buffer &buffer,
                    VmaAllocation &allocation,
                    VmaAllocationCreateFlags flags = 0);
  void createImage(uint32_t width, uint32_t height, VmaMemoryUsage memUsage,
                   vk::Format format, vk::ImageTiling tiling,
                   vk::ImageUsageFlags usage, vk::Image &image,
                   VmaAllocation &allocation);
  vk::ImageView createImageView(vk::Image image, vk::Format format,
                                vk::ImageAspectFlags aspectFlags);
  void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer,
                  vk::DeviceSize size);
  void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width,
                         uint32_t height);
  void updateUniformBuffers(uint32_t currentImage);
  void createTextureSampler();
  void transitionImageLayout(vk::Image image, vk::Format format,
                             vk::ImageLayout oldLayout,
                             vk::ImageLayout newLayout);

  SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice device);
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  bool isDeviceSuitable(vk::PhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
  bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

  vk::ShaderModule createShaderModule(const std::vector<char> &code);

  uint32_t findMemoryType(uint32_t typeFilter,
                          vk::MemoryPropertyFlags properties);

  vk::Instance instance;

  vk::DebugUtilsMessengerEXT debugMessenger;

  vk::PhysicalDevice physicalDevice;
  vk::Device device;

  vk::Queue graphicsQueue;
  vk::Queue presentQueue;

  vk::SurfaceKHR surface;

  vk::SwapchainKHR swapchain;
  vk::Format swapchainImageFormat;
  vk::Extent2D swapchainExtent;

  std::vector<vk::Image> swapchainImages;
  std::vector<vk::ImageView> swapchainImageViews;
  std::vector<vk::Framebuffer> swapchainFramebuffers;

  vk::Image depthImage;
  vk::ImageView depthImageView;
  VmaAllocation depthImageAllocation;

  vk::RenderPass renderPass;

  std::vector<UniformBuffer> globalUniformBuffers;
  std::vector<UniformBuffer> globalLightUniformBuffers;
  std::vector<vk::DescriptorSet> globalDescriptorSets;

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  vk::PipelineLayout pipelineLayout;

  vk::PipelineCache pipelineCache;
  std::unordered_map<std::string, vk::Pipeline> graphicsPipelines;
  std::vector<Pipeline> pipelineObjects;

  vk::Sampler textureSampler;

  vk::CommandPool commandPool;
  vk::CommandPool transferCommandPool;
  std::vector<vk::CommandBuffer> commandBuffers;

  std::vector<vk::Semaphore> imageAvailableSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  std::vector<vk::Fence> imagesInFlight;
  vk::Fence copyFinishedFence;

  VmaAllocator allocator;

  // Keeps track of all allocations in order to be freed
  // at end of runtime
  std::vector<IndexedVertexBuffer> indexedVertexBuffers;
  std::vector<Texture> textures;

  size_t currentFrame = 0;

  glm::vec4 clearColor{0.0f, 0.0f, 0.0f, 1.0f};

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  const size_t MAX_FRAMES_IN_FLIGHT = 2;

#ifndef ASH_DEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif
};

} // namespace Ash
