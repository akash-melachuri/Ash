#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core.h"
#include "Helper.h"
#include "Pipeline.h"

#define VULKAN_VERSION VK_API_VERSION_1_2

#define MAX_INSTANCES 1024

namespace Ash {

class VulkanAPI {
   public:
    VulkanAPI();
    ~VulkanAPI();

    void init(const std::vector<Pipeline>& pipelines);
    void render();
    void cleanup();

    void setClearColor(const glm::vec4& color);

    IndexedVertexBuffer createIndexedVertexArray(
        const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices);
    void createDescriptorSets();
    void createDescriptorSets(std::vector<VkDescriptorSet>& sets,
                              const Texture& texture);
    void createUniformBuffers(std::vector<UniformBuffer>& ubos);
    void createTextureImage(const std::string& path, Texture& texture);
    void createTextureImageView(Texture& texture);

   private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentsFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentsFamily.has_value();
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
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
    void createDescriptorSetLayout();
    void createPipelineCache();
    void createGraphicsPipelines(const std::vector<Pipeline>& pipelines);
    void createFramebuffers();
    void createDescriptorPool(uint32_t maxSets);
    void createCommandPools();
    void createCommandBuffers();
    void recordCommandBuffers();
    void createSyncObjects();
    void cleanupSwapchain();
    void recreateSwapchain();
    void updateCommandBuffers();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
    bool hasStencilComponent(VkFormat format);
    VkFormat findDepthFormat();
    void createDepthResources();
    void createBuffer(VkDeviceSize size, VmaMemoryUsage memUsage,
                      VkBufferUsageFlags usage, VkBuffer& buffer,
                      VmaAllocation& allocation);
    void createImage(uint32_t width, uint32_t height, VmaMemoryUsage memUsage,
                     VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkImage& image,
                     VmaAllocation& allocation);
    VkImageView createImageView(VkImage image, VkFormat format,
                                VkImageAspectFlags aspectFlags);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);
    void updateUniformBuffers(uint32_t currentImage);
    void createTextureSampler();
    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    VkInstance instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkImage depthImage;
    VkImageView depthImageView;
    VmaAllocation depthImageAllocation;

    VkRenderPass renderPass;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout imageDescriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    VkDescriptorPool descriptorPool;

    VkPipelineCache pipelineCache;
    std::unordered_map<std::string, VkPipeline> graphicsPipelines;
    std::vector<Pipeline> pipelineObjects;

    VkSampler textureSampler;

    VkCommandPool commandPool;
    VkCommandPool transferCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkDescriptorSet> uboDescriptorSets;
    std::vector<UniformBuffer> uniformBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    VkFence copyFinishedFence;

    VmaAllocator allocator;

    // Keeps track of all allocations in order to be freed
    // at end of runtime
    std::vector<IndexedVertexBuffer> indexedVertexBuffers;
    std::vector<Texture> textures;

    size_t currentFrame = 0;

    glm::vec4 clearColor{0.0f, 0.0f, 0.0f, 1.0f};

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const size_t MAX_FRAMES_IN_FLIGHT = 2;

#ifndef ASH_DEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};

}  // namespace Ash
