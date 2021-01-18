#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

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

#define MAX_DESCRIPTOR_SETS 10

namespace Ash {

class VulkanAPI {
   public:
    VulkanAPI();
    ~VulkanAPI();

    void init(const std::vector<Pipeline>& pipelines);
    void render();
    void cleanup();

    void setClearColor(const glm::vec4& color);
    void setPipeline(std::string name);
    void inline signalRecord() { shouldRecord = true; }

    void submitBatch(const std::unordered_map<std::string, Mesh>& batch);

    IndexedVertexBuffer createIndexedVertexArray(
        const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices);
    void createDescriptorSets(std::vector<VkDescriptorSet>& sets,
                              const std::vector<UniformBuffer>& ubo);
    void createUniformBuffers(std::vector<UniformBuffer>& ubos);

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
    void createBuffer(VkDeviceSize size, VmaMemoryUsage memUsage,
                      VkBufferUsageFlags usage, VkBuffer& buffer,
                      VmaAllocation& allocation);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void updateUniformBuffers(uint32_t currentImage);

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

    VkRenderPass renderPass;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkPipelineCache pipelineCache;
    std::string currentPipeline = "main";
    std::unordered_map<std::string, VkPipeline> graphicsPipelines;
    std::vector<Pipeline> pipelineObjects;

    std::unordered_map<std::string, Mesh> batch;

    // When true, means command buffers need to be re-recorded because they are
    // outdated Usually means new object/change in rendering properties e.g.
    // changing clear color or shaders
    bool shouldRecord = false;
    VkCommandPool commandPool;
    VkCommandPool transferCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    VkFence copyFinishedFence;

    VmaAllocator allocator;

    std::vector<IndexedVertexBuffer> indexedVertexBuffers;
    std::vector<UniformBuffer> uniformBuffers;

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
