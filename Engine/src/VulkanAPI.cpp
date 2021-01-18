#include "VulkanAPI.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "App.h"
#include "Components.h"
#include "Renderer.h"

namespace Ash {

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    } else {
        ASH_ERROR("Couldn't load function vkDestroyDebugUtilsMessengerEXT");
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::string type;
        switch (messageType) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                type = "General";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                type = "Specification Violation";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                type = "Performance";
                break;
            default:
                type = "Other";
                break;
        }
        ASH_ERROR("{} Type Validation Layer: {} ", type,
                  pCallbackData->pMessage);
        if (pUserData) {
            ASH_WARN("User data is provided");
        }
    }
    return VK_FALSE;
}

VulkanAPI::VulkanAPI() {}

VulkanAPI::~VulkanAPI() {}

VulkanAPI::QueueFamilyIndices VulkanAPI::findQueueFamilies(
    VkPhysicalDevice device) {
    VulkanAPI::QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);

        if (presentSupport) {
            indices.presentsFamily = i;
        }

        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}

VulkanAPI::SwapchainSupportDetails VulkanAPI::querySwapchainSupport(
    VkPhysicalDevice device) {
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                             details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanAPI::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    // Consider ranking formats and choosing best option if desired format isn't
    // found
    return availableFormats[0];
}

VkPresentModeKHR VulkanAPI::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        // vsync : VK_PRESENT_MODE_MAILBOX_KHR
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            return availablePresentMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanAPI::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(App::getWindow()->get(), &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                                   static_cast<uint32_t>(height)};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallBack;
}

int rateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    int score = 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

bool VulkanAPI::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionsCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanAPI::isDeviceSuitable(VkPhysicalDevice device) {
    VulkanAPI::QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapChainSupport =
            querySwapchainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
                            !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void VulkanAPI::pickPhysicalDevice() {
    physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    ASH_ASSERT(deviceCount != 0, "No devices with Vulkan support found");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            ASH_INFO("Selecting {} as the physical device",
                     properties.deviceName);
            break;
        }
    }

    ASH_ASSERT(physicalDevice != VK_NULL_HANDLE, "No suitable device found");
}

void VulkanAPI::createInstance() {
    if (enableValidationLayers && !checkValidationSupport())
        ASH_ERROR("Enabled validation layers, but not supported");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Game";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "Ash";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        createInfo.enabledExtensionCount =
            static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        ASH_INFO("Enabling validation layers");
    } else {
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    ASH_ASSERT(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS,
               "Failed to initialize vulkan");

    ASH_INFO("Initialized Vulkan instance");
}

void VulkanAPI::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    ASH_ASSERT(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                            &debugMessenger) == VK_SUCCESS,
               "Failed to setup Debug Messenger");
}

void VulkanAPI::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentsFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    ASH_ASSERT(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) ==
                   VK_SUCCESS,
               "Couldn't create logical device");

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentsFamily.value(), 0, &presentQueue);
}

void VulkanAPI::createAllocator() {
    VmaAllocatorCreateInfo allocInfo{};
    allocInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocInfo.device = device;
    allocInfo.physicalDevice = physicalDevice;
    allocInfo.instance = instance;

    ASH_ASSERT(vmaCreateAllocator(&allocInfo, &allocator) == VK_SUCCESS,
               "Failed to create allocator");
}

void VulkanAPI::createSwapchain() {
    ASH_INFO("Creating swapchain");
    SwapchainSupportDetails swapchainSupport =
        querySwapchainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode =
        chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndicies[] = {indices.graphicsFamily.value(),
                                      indices.presentsFamily.value()};
    if (indices.graphicsFamily != indices.presentsFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndicies;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    ASH_ASSERT(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) ==
                   VK_SUCCESS,
               "Failed to create swapchain");

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount,
                            swapchainImages.data());

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;
}

void VulkanAPI::createImageViews() {
    ASH_INFO("Creating image views");
    swapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        ASH_ASSERT(vkCreateImageView(device, &createInfo, nullptr,
                                     &swapchainImageViews[i]) == VK_SUCCESS,
                   "Failed to create swapchain image view {}", i);
    }
}

void VulkanAPI::createRenderPass() {
    ASH_INFO("Creating render pass");
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    ASH_ASSERT(vkCreateRenderPass(device, &renderPassInfo, nullptr,
                                  &renderPass) == VK_SUCCESS,
               "Failed to create render pass");
}

void VulkanAPI::createDescriptorSetLayout() {
    ASH_INFO("Creating descriptor set layout");
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    ASH_ASSERT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                           &descriptorSetLayout) == VK_SUCCESS,
               "Failed to create descriptor set layout");
}

void VulkanAPI::createPipelineCache() {
    ASH_INFO("Creating pipeline cache");
    VkPipelineCacheCreateInfo cacheInfo{};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    ASH_ASSERT(vkCreatePipelineCache(device, &cacheInfo, nullptr,
                                     &pipelineCache) == VK_SUCCESS,
               "Failed to create pipeline cache");
}

void VulkanAPI::createGraphicsPipelines(
    const std::vector<Pipeline>& pipelines) {
    ASH_INFO("Creating graphics pipelines");

    pipelineObjects = pipelines;

    std::vector<char> vert =
        Helper::readBinaryFile("assets/shaders/shader.vert.spv");
    std::vector<char> frag =
        Helper::readBinaryFile("assets/shaders/shader.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vert);
    VkShaderModule fragShaderModule = createShaderModule(frag);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
    // VK_DYNAMIC_STATE_LINE_WIDTH};

    // VkPipelineDynamicStateCreateInfo dynamicState{};
    // dynamicState.sType =
    // VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    // dynamicState.dynamicStateCount = 2;
    // dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    ASH_ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                                      &pipelineLayout) == VK_SUCCESS,
               "Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipelineLayout;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    ASH_ASSERT(vkCreateGraphicsPipelines(
                   device, pipelineCache, 1, &pipelineInfo, nullptr,
                   &graphicsPipelines["main"]) == VK_SUCCESS,
               "Failed to create graphics pipeline");

    pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    pipelineInfo.basePipelineHandle = graphicsPipelines["main"];
    pipelineInfo.basePipelineIndex = -1;

    size_t j = 0;
    for (const Pipeline& pipeline : pipelines) {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos;
        std::vector<VkShaderModule> shaderModules;
        for (size_t i = 0; i < pipeline.stages.size(); i++) {
            std::vector<char> code =
                Helper::readBinaryFile(pipeline.paths[i].c_str());
            shaderModules.push_back(createShaderModule(code));

            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

            switch (pipeline.stages[i]) {
                case ShaderStages::VERTEX_SHADER_STAGE:
                    shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    break;
                case ShaderStages::FRAGMENT_SHADER_STAGE:
                    shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    break;
            }

            shaderStageInfo.module = shaderModules.back();
            shaderStageInfo.pName = "main";
            shaderStageInfo.pSpecializationInfo = nullptr;
            shaderStageInfos.push_back(shaderStageInfo);
        }

        pipelineInfo.stageCount = static_cast<uint32_t>(pipeline.stages.size());
        pipelineInfo.pStages = shaderStageInfos.data();

        ASH_ASSERT(vkCreateGraphicsPipelines(
                       device, pipelineCache, 1, &pipelineInfo, nullptr,
                       &graphicsPipelines[pipeline.name]) == VK_SUCCESS,
                   "Failed to create user pipeline");

        for (auto& module : shaderModules)
            vkDestroyShaderModule(device, module, nullptr);

        j++;
    }

    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void VulkanAPI::createFramebuffers() {
    ASH_INFO("Creating framebuffers");

    swapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {swapchainImageViews[i]};

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = renderPass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = swapchainExtent.width;
        frameBufferInfo.height = swapchainExtent.height;
        frameBufferInfo.layers = 1;

        ASH_ASSERT(vkCreateFramebuffer(device, &frameBufferInfo, nullptr,
                                       &swapchainFramebuffers[i]) == VK_SUCCESS,
                   "Failed to create framebuffer {}", i);
    }
}

void VulkanAPI::createUniformBuffers(std::vector<UniformBuffer>& ubos) {
    ASH_INFO("Creating uniform buffers");

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    ubos.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        createBuffer(bufferSize, VMA_MEMORY_USAGE_CPU_TO_GPU,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, ubos[i].uniformBuffer,
                     ubos[i].uniformBufferAllocation);
    }
}

void VulkanAPI::createDescriptorPool(uint32_t maxSets) {
    ASH_INFO("Creating descriptor pool");
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount =
        static_cast<uint32_t>(swapchainImages.size() * maxSets);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapchainImages.size() * maxSets);

    ASH_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr,
                                      &descriptorPool) == VK_SUCCESS,
               "Failed to create descriptor pool");
}

void VulkanAPI::createDescriptorSets(std::vector<VkDescriptorSet>& sets,
                                     const std::vector<UniformBuffer>& ubo) {
    ASH_INFO("Creating descriptor sets");
    std::vector<VkDescriptorSetLayout> layouts(swapchainImages.size(),
                                               descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(swapchainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    sets.resize(swapchainImages.size());
    ASH_ASSERT(
        vkAllocateDescriptorSets(device, &allocInfo, sets.data()) == VK_SUCCESS,
        "Failed to allocate descriptor sets");

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = ubo[i].uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = sets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanAPI::createCommandPools() {
    ASH_INFO("Creating command pool");
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0;

    ASH_ASSERT(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) ==
                   VK_SUCCESS,
               "Failed to create command pool");

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    ASH_ASSERT(vkCreateCommandPool(device, &poolInfo, nullptr,
                                   &transferCommandPool) == VK_SUCCESS,
               "Failed to create command pool");
}

uint32_t VulkanAPI::findMemoryType(uint32_t typeFilter,
                                   VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties)
            return i;
    }

    ASH_ASSERT(false, "Failed to find suitable memory type");
}

void VulkanAPI::createCommandBuffers() {
    ASH_INFO("Creating command buffers");

    commandBuffers.resize(swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    ASH_ASSERT(vkAllocateCommandBuffers(device, &allocInfo,
                                        commandBuffers.data()) == VK_SUCCESS,
               "Failed to allocate command buffers");

    recordCommandBuffers();
}

void VulkanAPI::recordCommandBuffers() {
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        ASH_ASSERT(
            vkBeginCommandBuffer(commandBuffers[i], &beginInfo) == VK_SUCCESS,
            "Failed to begin command buffer {}", i);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchainExtent;

        VkClearValue clearColor = {{{this->clearColor.r, this->clearColor.g,
                                     this->clearColor.b, this->clearColor.a}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        // Each model should have their own pipeline
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphicsPipelines[currentPipeline]);

        VkDeviceSize offsets[] = {0};

        std::shared_ptr<Scene> scene = Renderer::getScene();
        if (scene) {
            auto renderables = scene->registry.view<Renderable>();

            for (auto entity : renderables) {
                auto& renderable = renderables.get(entity);

                Mesh& mesh = Renderer::getMesh(renderable.mesh);
                VkBuffer vb[] = {mesh.ivb.buffer};

                // Each model has their own mesh and thus their own vertex and
                // index buffers
                vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vb, offsets);

                vkCmdBindIndexBuffer(commandBuffers[i], mesh.ivb.buffer,
                                     mesh.ivb.vertSize, VK_INDEX_TYPE_UINT32);

                // vkUpdateDescriptorSets ? look up if this is slow/fast
                // otherwise use unique descriptor sets for every entity - seems
                // wasteful

                // Each entity has their own transform and thus their own
                // UBO transform matrix
                ASH_INFO("Using descriptor set {}", i);
                vkCmdBindDescriptorSets(
                    commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, 1, &renderable.descriptorSets[i], 0,
                    nullptr);

                vkCmdDrawIndexed(commandBuffers[i], mesh.ivb.numIndices, 1, 0,
                                 0, 0);
            }
        }

        // for (auto mesh : batch) {
        //     VkBuffer vb[] = {mesh.second.ivb.buffer};

        //     // Each model has their own mesh and thus their own vertex and
        //     index
        //     // buffers
        //     vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vb, offsets);

        //     vkCmdBindIndexBuffer(commandBuffers[i], mesh.second.ivb.buffer,
        //                          mesh.second.ivb.vertSize,
        //                          VK_INDEX_TYPE_UINT32);

        //     // vkUpdateDescriptorSets ? look up if this is slow/fast
        //     // otherwise use unique descriptor sets for every entity - seems
        //     // wasteful

        //     // Each entity has their own transform and thus their own
        //     // UBO transform matrix
        //     vkCmdBindDescriptorSets(
        //         commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
        //         pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        //     vkCmdDrawIndexed(commandBuffers[i], mesh.second.ivb.numIndices,
        //     1,
        //                      0, 0, 0);
        // }

        vkCmdEndRenderPass(commandBuffers[i]);

        ASH_ASSERT(vkEndCommandBuffer(commandBuffers[i]) == VK_SUCCESS,
                   "Failed to record command buffer {}", i);
    }

    shouldRecord = false;
}

void VulkanAPI::createSyncObjects() {
    ASH_INFO("Creating synchronization objects");

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        ASH_ASSERT(
            vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                              &imageAvailableSemaphores[i]) == VK_SUCCESS,
            "Failed to create semaphore");
        ASH_ASSERT(
            vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                              &renderFinishedSemaphores[i]) == VK_SUCCESS,
            "Failed to create semaphore");
        ASH_ASSERT(vkCreateFence(device, &fenceInfo, nullptr,
                                 &inFlightFences[i]) == VK_SUCCESS,
                   "Failed to create fence");
    }

    ASH_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &copyFinishedFence) ==
                   VK_SUCCESS,
               "Failed to create fence");
}

void VulkanAPI::cleanupSwapchain() {
    for (auto framebuffer : swapchainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    vkFreeCommandBuffers(device, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());

    for (auto pipeline : graphicsPipelines)
        vkDestroyPipeline(device, pipeline.second, nullptr);

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto imageView : swapchainImageViews)
        vkDestroyImageView(device, imageView, nullptr);

    vkDestroySwapchainKHR(device, swapchain, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void VulkanAPI::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(App::getWindow()->get(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(App::getWindow()->get(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    ASH_INFO("Recreating swapchain");

    cleanupSwapchain();

    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipelines(pipelineObjects);
    createFramebuffers();
    createDescriptorPool(MAX_DESCRIPTOR_SETS);

    std::shared_ptr<Scene> scene = Renderer::getScene();
    auto renderables = scene->registry.view<Renderable>();
    for (auto entity : renderables) {
        auto& renderable = renderables.get(entity);
        createDescriptorSets(renderable.descriptorSets, renderable.ubos);
    }

    // delete
    createDescriptorSets(descriptorSets, uniformBuffers);
    createCommandBuffers();
}

void VulkanAPI::createBuffer(VkDeviceSize size, VmaMemoryUsage memUsage,
                             VkBufferUsageFlags usage, VkBuffer& buffer,
                             VmaAllocation& allocation) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = memUsage;

    ASH_ASSERT(vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, &buffer,
                               &allocation, nullptr) == VK_SUCCESS,
               "Failed to create buffer and allocation");
}

void VulkanAPI::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                           VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = transferCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    ASH_ASSERT(vkResetFences(device, 1, &copyFinishedFence) == VK_SUCCESS,
               "Failed to reset copy finished fence");

    ASH_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                             copyFinishedFence) == VK_SUCCESS,
               "Failed to submit copy command buffer");

    vkWaitForFences(device, 1, &copyFinishedFence, VK_TRUE, UINT64_MAX);

    vkFreeCommandBuffers(device, transferCommandPool, 1, &commandBuffer);
}

VkShaderModule VulkanAPI::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule module;
    ASH_ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &module) ==
                   VK_SUCCESS,
               "Failed to create shader module");

    return module;
}

void VulkanAPI::createSurface() {
    GLFWwindow* window = Ash::App::getWindow()->get();
    VkResult result =
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
    ASH_ASSERT(result == VK_SUCCESS, "Failed to create window surface, {}",
               result);

    ASH_INFO("Created Vulkan surface");
}

void VulkanAPI::init(const std::vector<Pipeline>& pipelines) {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createPipelineCache();
    createDescriptorSetLayout();
    createGraphicsPipelines(pipelines);
    createFramebuffers();
    // Delete
    createUniformBuffers(uniformBuffers);
    createDescriptorPool(MAX_DESCRIPTOR_SETS);
    // Delete
    createDescriptorSets(descriptorSets, uniformBuffers);
    createCommandPools();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanAPI::updateUniformBuffers(uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view =
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(
        glm::radians(45.0f),
        swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f);

    ubo.proj[1][1] *= -1;

    std::shared_ptr<Scene> scene = Renderer::getScene();
    if (scene) {
        auto renderables = scene->registry.view<Renderable>();
        for (auto entity : renderables) {
            auto& renderable = renderables.get(entity);
            void* data;
            vmaMapMemory(allocator,
                         renderable.ubos[currentImage].uniformBufferAllocation,
                         &data);
            std::memcpy(data, &ubo, sizeof(ubo));
            vmaUnmapMemory(
                allocator,
                renderable.ubos[currentImage].uniformBufferAllocation);
        }
    }
}

void VulkanAPI::render() {
    if (shouldRecord) updateCommandBuffers();

    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE,
                    UINT64_MAX);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }

    ASH_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
               "Failed to acquire swapchain image");

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE,
                        UINT64_MAX);

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniformBuffers(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    ASH_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                             inFlightFences[currentFrame]) == VK_SUCCESS,
               "Failed to submit render command buffer");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        App::getWindow()->framebufferResized) {
        App::getWindow()->framebufferResized = false;
        recreateSwapchain();
    } else {
        ASH_ASSERT(result == VK_SUCCESS, "Failed to present swapchain image");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanAPI::cleanup() {
    vkDeviceWaitIdle(device);

    ASH_INFO("Cleaning up graphics API");

    vkDestroyPipelineCache(device, pipelineCache, nullptr);

    cleanupSwapchain();

    for (auto buffer : uniformBuffers) {
        vmaDestroyBuffer(allocator, buffer.uniformBuffer,
                         buffer.uniformBufferAllocation);
    }

    std::shared_ptr<Scene> scene = Renderer::getScene();
    if (scene) {
        auto renderables = scene->registry.view<Renderable>();
        for (auto entity : renderables) {
            auto& renderable = renderables.get<Renderable>(entity);
            for (auto buffer : renderable.ubos) {
                vmaDestroyBuffer(allocator, buffer.uniformBuffer,
                                 buffer.uniformBufferAllocation);
            }
        }
    }

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    for (IndexedVertexBuffer ivb : indexedVertexBuffers) {
        vmaDestroyBuffer(allocator, ivb.buffer, ivb.bufferAllocation);
    }

    vmaDestroyAllocator(allocator);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyFence(device, copyFinishedFence, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyCommandPool(device, transferCommandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanAPI::updateCommandBuffers() {
    vkQueueWaitIdle(graphicsQueue);
    vkResetCommandPool(device, commandPool, 0);
    recordCommandBuffers();
}

/*
 *
 *      Renderer API
 *
 */

void VulkanAPI::setPipeline(std::string name) {
    currentPipeline = name;

    shouldRecord = true;
}

void VulkanAPI::setClearColor(const glm::vec4& color) {
    clearColor = color;

    shouldRecord = true;
}

void VulkanAPI::submitBatch(
    const std::unordered_map<std::string, Mesh>& batch) {
    this->batch = batch;

    shouldRecord = true;
}

IndexedVertexBuffer VulkanAPI::createIndexedVertexArray(
    const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices) {
    IndexedVertexBuffer ret{};
    ret.numIndices = indices.size();

    VkDeviceSize vertSize = sizeof(verts[0]) * verts.size();
    ret.vertSize = vertSize;

    VkDeviceSize indicesSize = sizeof(indices[0]) * indices.size();
    VkDeviceSize bufferSize = vertSize + indicesSize;

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    createBuffer(bufferSize, VMA_MEMORY_USAGE_CPU_ONLY,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer,
                 stagingBufferAllocation);

    void* data;
    vmaMapMemory(allocator, stagingBufferAllocation, &data);
    std::memcpy(data, verts.data(), static_cast<size_t>(vertSize));
    std::memcpy(static_cast<Vertex*>(data) + verts.size(), indices.data(),
                static_cast<size_t>(indicesSize));
    vmaUnmapMemory(allocator, stagingBufferAllocation);

    createBuffer(bufferSize, VMA_MEMORY_USAGE_GPU_ONLY,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 ret.buffer, ret.bufferAllocation);

    copyBuffer(stagingBuffer, ret.buffer, bufferSize);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);

    indexedVertexBuffers.push_back(ret);

    return ret;
}

/*
 *
 *      Renderer API
 *
 */

bool VulkanAPI::checkValidationSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

}  // namespace Ash
