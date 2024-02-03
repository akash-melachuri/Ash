#include "VulkanAPI.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include "App.h"
#include "Components.h"
#include "Renderer.h"

namespace Ash {

vk::CommandBuffer VulkanAPI::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo(commandPool,
                                          vk::CommandBufferLevel::ePrimary, 1);

  vk::CommandBuffer commandBuffer =
      device.allocateCommandBuffers(allocInfo).front();

  vk::CommandBufferBeginInfo beginInfo(
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

void VulkanAPI::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
  commandBuffer.end();

  vk::SubmitInfo submitInfo(nullptr, nullptr, commandBuffer);

  graphicsQueue.submit(submitInfo);
  graphicsQueue.waitIdle();

  device.freeCommandBuffers(commandPool, commandBuffer);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
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
    ASH_ERROR("{} Type Validation Layer: {} ", type, pCallbackData->pMessage);
    if (pUserData) {
      ASH_WARN("User data is provided");
    }
  }
  return VK_FALSE;
}

VulkanAPI::VulkanAPI() {}

VulkanAPI::~VulkanAPI() {}

VulkanAPI::QueueFamilyIndices
VulkanAPI::findQueueFamilies(vk::PhysicalDevice device) {
  VulkanAPI::QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

  std::vector<vk::QueueFamilyProperties> queueFamilies =
      device.getQueueFamilyProperties();

  uint32_t i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphicsFamily = i;
    }

    vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i, surface);

    if (presentSupport) {
      indices.presentsFamily = i;
    }

    if (indices.isComplete())
      break;

    i++;
  }

  return indices;
}

VulkanAPI::SwapchainSupportDetails
VulkanAPI::querySwapchainSupport(vk::PhysicalDevice device) {
  SwapchainSupportDetails details;
  details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
  details.formats = device.getSurfaceFormatsKHR(surface);
  details.presentModes = device.getSurfacePresentModesKHR(surface);
  return details;
}

vk::SurfaceFormatKHR VulkanAPI::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  // Consider ranking formats and choosing best option if desired format isn't
  // found
  return availableFormats[0];
}

vk::PresentModeKHR VulkanAPI::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    // vsync : VK_PRESENT_MODE_MAILBOX_KHR
    if (availablePresentMode == vk::PresentModeKHR::eImmediate)
      return availablePresentMode;
    else if (availablePresentMode == vk::PresentModeKHR::eMailbox)
      return availablePresentMode;
  }
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
VulkanAPI::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(App::getWindow()->get(), &width, &height);

    vk::Extent2D actualExtent = {static_cast<uint32_t>(width),
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
    vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
      {},
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      debugCallBack);
}

int rateDeviceSuitability(vk::PhysicalDevice device) {
  vk::PhysicalDeviceProperties deviceProperties = device.getProperties();

  int score = 0;

  if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    score += 1000;
  }

  score += deviceProperties.limits.maxImageDimension2D;

  return score;
}

bool VulkanAPI::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
  std::vector<vk::ExtensionProperties> availableExtensions =
      device.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

bool VulkanAPI::isDeviceSuitable(vk::PhysicalDevice device) {
  VulkanAPI::QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapchainSupportDetails swapChainSupport = querySwapchainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();

  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

void VulkanAPI::pickPhysicalDevice() {
  physicalDevice = VK_NULL_HANDLE;

  std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
  ASH_ASSERT(!devices.empty(), "No devices with Vulkan support found");

  std::multimap<int, vk::PhysicalDevice> candidates;

  int maxScore{0};
  vk::PhysicalDevice bestDevice{devices[0]};
  for (const auto &device : devices) {
    if (rateDeviceSuitability(device) > maxScore && isDeviceSuitable(device)) {
      maxScore = rateDeviceSuitability(device);
      bestDevice = device;
    }
  }

  physicalDevice = bestDevice;
  vk::PhysicalDeviceProperties properties = bestDevice.getProperties();
  ASH_INFO("Selecting {} as the physical device", properties.deviceName);

  ASH_ASSERT(physicalDevice != VK_NULL_HANDLE, "No suitable device found");
}

void VulkanAPI::createInstance() {
  if (enableValidationLayers && !checkValidationSupport())
    ASH_ERROR("Enabled validation layers, but not supported");

  vk::ApplicationInfo appInfo("Game", VK_MAKE_VERSION(0, 0, 1), "Ash",
                              VK_MAKE_VERSION(0, 0, 1), VULKAN_VERSION);

  vk::InstanceCreateInfo createInfo({}, &appInfo);

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    createInfo.setPEnabledExtensionNames(extensions);

    ASH_INFO("Enabling validation layers");
  } else {
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
  }

  vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enableValidationLayers) {
    createInfo.setPEnabledLayerNames(validationLayers);

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = &debugCreateInfo;
  }

  instance = vk::createInstance(createInfo);

  ASH_INFO("Initialized Vulkan instance");
}

void VulkanAPI::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;

  vk::DebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

  vkCreateDebugUtilsMessengerEXT(
      instance,
      reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&createInfo),
      nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT *>(&debugMessenger));
}

void VulkanAPI::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentsFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo({}, queueFamily);
    queueCreateInfo.setQueuePriorities(queuePriority);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  vk::DeviceCreateInfo createInfo({}, queueCreateInfos, {}, deviceExtensions,
                                  &deviceFeatures);
  if (enableValidationLayers) {
    createInfo.setPEnabledLayerNames(validationLayers);
  }

  device = physicalDevice.createDevice(createInfo);

  graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
  presentQueue = device.getQueue(indices.presentsFamily.value(), 0);
}

void VulkanAPI::createAllocator() {
  VmaAllocatorCreateInfo allocInfo{};
  allocInfo.vulkanApiVersion = VULKAN_VERSION;
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

  vk::SurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapchainSupport.formats);
  vk::PresentModeKHR presentMode =
      chooseSwapPresentMode(swapchainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

  uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
  if (swapchainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapchainSupport.capabilities.maxImageCount) {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo(
      {}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
      extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  uint32_t queueFamilyIndicies[] = {indices.graphicsFamily.value(),
                                    indices.presentsFamily.value()};
  if (indices.graphicsFamily != indices.presentsFamily) {
    createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
    createInfo.setQueueFamilyIndices(queueFamilyIndicies);
  } else {
    createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
  }

  createInfo.setPreTransform(swapchainSupport.capabilities.currentTransform);
  createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  createInfo.setPresentMode(presentMode);
  createInfo.setClipped(VK_TRUE);
  createInfo.setOldSwapchain(VK_NULL_HANDLE);

  swapchain = device.createSwapchainKHR(createInfo);

  swapchainImages = device.getSwapchainImagesKHR(swapchain);
  swapchainImageFormat = surfaceFormat.format;
  swapchainExtent = extent;
}

void VulkanAPI::createImageViews() {
  ASH_INFO("Creating image views");
  swapchainImageViews.resize(swapchainImages.size());
  for (size_t i = 0; i < swapchainImages.size(); i++) {
    swapchainImageViews[i] =
        createImageView(swapchainImages[i], swapchainImageFormat,
                        vk::ImageAspectFlagBits::eColor);
  }
}

void VulkanAPI::createRenderPass() {
  ASH_INFO("Creating render pass");

  vk::AttachmentDescription colorAttachment({}, swapchainImageFormat,
                                            vk::SampleCountFlagBits::e1,
                                            vk::AttachmentLoadOp::eClear);
  colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
  colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
  colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
  colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentDescription depthAttachment({}, findDepthFormat());
  depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
  depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
  depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
  depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
  depthAttachment.setFinalLayout(
      vk::ImageLayout::eDepthStencilAttachmentOptimal);

  vk::AttachmentReference colorAttachmentRef(
      0, vk::ImageLayout::eColorAttachmentOptimal);

  vk::AttachmentReference depthAttachmentRef(
      1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

  vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {},
                                 colorAttachmentRef, {}, &depthAttachmentRef);

  vk::SubpassDependency dependency(
      VK_SUBPASS_EXTERNAL, 0,
      vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
      vk::PipelineStageFlagBits::eColorAttachmentOutput |
          vk::PipelineStageFlagBits::eEarlyFragmentTests,
      {},
      vk::AccessFlagBits::eColorAttachmentWrite |
          vk::AccessFlagBits::eDepthStencilAttachmentWrite);

  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment,
                                                          depthAttachment};

  vk::RenderPassCreateInfo renderPassInfo({}, attachments, subpass, dependency);

  renderPass = device.createRenderPass(renderPassInfo);
}

void VulkanAPI::createDescriptorSetLayouts() {
  ASH_INFO("Creating descriptor set layout");

  descriptorLayoutCache.init(device);

  vk::DescriptorSetLayoutBinding gboLayoutBinding(
      0, vk::DescriptorType::eUniformBuffer, 1,
      vk::ShaderStageFlagBits::eVertex);

  vk::DescriptorSetLayoutBinding lboLayoutBinding(
      1, vk::DescriptorType::eUniformBuffer, 1,
      vk::ShaderStageFlagBits::eFragment);

  std::array<vk::DescriptorSetLayoutBinding, 2> globalBindings = {
      gboLayoutBinding, lboLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo globalLayoutInfo({}, globalBindings);

  vk::DescriptorSetLayoutBinding materialDiffuseSamplerLayoutBinding(
      0, vk::DescriptorType::eCombinedImageSampler, 1,
      vk::ShaderStageFlagBits::eFragment);

  std::array<vk::DescriptorSetLayoutBinding, 1> materialBindings = {
      materialDiffuseSamplerLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo materialLayoutInfo({}, materialBindings);

  vk::DescriptorSetLayoutBinding objectUboLayoutBinding(
      0, vk::DescriptorType::eUniformBuffer, 1,
      vk::ShaderStageFlagBits::eVertex);

  std::array<vk::DescriptorSetLayoutBinding, 1> objectBindings = {
      objectUboLayoutBinding};

  vk::DescriptorSetLayoutCreateInfo objectLayoutInfo({}, objectBindings);

  descriptorSetLayouts.push_back(
      descriptorLayoutCache.create_descriptor_layout(globalLayoutInfo));

  descriptorSetLayouts.push_back(
      descriptorLayoutCache.create_descriptor_layout(materialLayoutInfo));

  descriptorSetLayouts.push_back(
      descriptorLayoutCache.create_descriptor_layout(objectLayoutInfo));
}

void VulkanAPI::createPipelineCache() {
  ASH_INFO("Creating pipeline cache");
  pipelineCache = device.createPipelineCache({});
}

void VulkanAPI::createGraphicsPipelines(
    const std::vector<Pipeline> &pipelines) {
  ASH_INFO("Creating graphics pipelines");

  pipelineObjects = pipelines;

  std::vector<char> vert =
      Helper::readBinaryFile("assets/shaders/shader.vert.spv");
  std::vector<char> frag =
      Helper::readBinaryFile("assets/shaders/shader.frag.spv");

  vk::ShaderModule vertShaderModule = createShaderModule(vert);
  vk::ShaderModule fragShaderModule = createShaderModule(frag);

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
      {}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
      {}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");

  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescription();

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, bindingDescription,
                                                         attributeDescriptions);

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
      {}, vk::PrimitiveTopology::eTriangleList, vk::False);

  vk::PipelineViewportStateCreateInfo viewportState({}, 1, {}, 1, {});

  vk::PipelineRasterizationStateCreateInfo rasterizer(
      {}, vk::False, vk::False, vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False,
      0, 0, 0, 1);

  vk::PipelineMultisampleStateCreateInfo multisampling(
      {}, vk::SampleCountFlagBits::e1, vk::False, 1, {}, vk::False, vk::False);

  vk::PipelineColorBlendAttachmentState colorBlendAttachment(
      vk::True, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha,
      vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero,
      vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  vk::PipelineColorBlendStateCreateInfo colorBlending(
      {}, vk::False, vk::LogicOp::eCopy, colorBlendAttachment);

  vk::DynamicState dynamicStates[] = {vk::DynamicState::eViewport,
                                      vk::DynamicState::eScissor};

  vk::PipelineDepthStencilStateCreateInfo depthStencil(
      {}, vk::True, vk::True, vk::CompareOp::eLess, vk::False, vk::False);

  vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, descriptorSetLayouts);

  pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);

  vk::GraphicsPipelineCreateInfo pipelineInfo(
      vk::PipelineCreateFlagBits::eAllowDerivatives);

  pipelineInfo.setStages(shaderStages);

  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;

  pipelineInfo.layout = pipelineLayout;

  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;

  auto [result, graphicsPipeline] =
      device.createGraphicsPipelines(pipelineCache, pipelineInfo);

  ASH_ASSERT(result == vk::Result::eSuccess,
             "Failed to create graphics pipeline");
  graphicsPipelines["main"] = graphicsPipeline.front();

  pipelineInfo.flags = vk::PipelineCreateFlagBits::eDerivative;
  pipelineInfo.basePipelineHandle = graphicsPipelines["main"];
  pipelineInfo.basePipelineIndex = -1;

  for (const Pipeline &pipeline : pipelines) {
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;
    std::vector<vk::ShaderModule> shaderModules;
    for (size_t i = 0; i < pipeline.stages.size(); i++) {
      std::vector<char> code =
          Helper::readBinaryFile(pipeline.paths[i].c_str());
      shaderModules.push_back(createShaderModule(code));

      vk::PipelineShaderStageCreateInfo shaderStageInfo;

      switch (pipeline.stages[i]) {
      case ShaderStages::VERTEX_SHADER_STAGE:
        shaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        break;
      case ShaderStages::FRAGMENT_SHADER_STAGE:
        shaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        break;
      }

      shaderStageInfo.module = shaderModules.back();
      shaderStageInfo.pName = "main";
      shaderStageInfo.pSpecializationInfo = nullptr;
      shaderStageInfos.push_back(shaderStageInfo);
    }

    pipelineInfo.setStages(shaderStageInfos);

    auto [result, pl] =
        device.createGraphicsPipelines(pipelineCache, pipelineInfo);
    ASH_ASSERT(result == vk::Result::eSuccess,
               "Failed to create user pipeline");
    graphicsPipelines[pipeline.name] = pl.front();

    for (auto &module : shaderModules)
      device.destroyShaderModule(module);
  }

  device.destroyShaderModule(vertShaderModule);
  device.destroyShaderModule(fragShaderModule);
}

void VulkanAPI::createFramebuffers() {
  ASH_INFO("Creating framebuffers");

  swapchainFramebuffers.resize(swapchainImageViews.size());

  for (size_t i = 0; i < swapchainImageViews.size(); i++) {
    std::array<vk::ImageView, 2> attachments = {swapchainImageViews[i],
                                                depthImageView};

    vk::FramebufferCreateInfo frameBufferInfo({}, renderPass, attachments,
                                              swapchainExtent.width,
                                              swapchainExtent.height, 1);

    swapchainFramebuffers[i] = device.createFramebuffer(frameBufferInfo);
  }
}

void VulkanAPI::createUniformBuffers(std::vector<UniformBuffer> &ubos,
                                     vk::DeviceSize bufferSize) {
  ubos.resize(swapchainImages.size());

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    createBuffer(bufferSize, VMA_MEMORY_USAGE_AUTO,
                 vk::BufferUsageFlagBits::eUniformBuffer, ubos[i].uniformBuffer,
                 ubos[i].uniformBufferAllocation,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
  }
}

void VulkanAPI::createDescriptorAllocator() {
  ASH_INFO("Creating descriptor allocator");

  descriptorAllocator.init(device);
}
void VulkanAPI::createGlobalDescriptorSets() {
  ASH_INFO("Creating global descriptor set for objects");

  globalDescriptorSets.resize(swapchainImages.size());
  for (size_t i = 0; i < swapchainImages.size(); i++) {
    vk::DescriptorBufferInfo bufferInfo(globalUniformBuffers[i].uniformBuffer,
                                        0, sizeof(GlobalBufferObject));

    vk::DescriptorBufferInfo lightBufferInfo(
        globalLightUniformBuffers[i].uniformBuffer, 0,
        sizeof(LightBufferObject));

    DescriptorBuilder::begin(&Renderer::getAPI()->descriptorLayoutCache,
                             &Renderer::getAPI()->descriptorAllocator)
        .bind_buffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer,
                     vk::ShaderStageFlagBits::eVertex)
        .bind_buffer(1, &lightBufferInfo, vk::DescriptorType::eUniformBuffer,
                     vk::ShaderStageFlagBits::eFragment)
        .build(globalDescriptorSets[i]);
  }
}

void VulkanAPI::createRenderableDescriptorSets(
    std::vector<vk::DescriptorSet> &sets, const std::vector<UniformBuffer> &ubo,
    Material &material) {
  ASH_INFO("Creating descriptor sets for objects and their materials");

  sets.resize(swapchainImages.size());
  material.sets.resize(swapchainImages.size());
  for (size_t i = 0; i < swapchainImages.size(); i++) {
    vk::DescriptorBufferInfo bufferInfo(ubo[i].uniformBuffer, 0,
                                        sizeof(RenderableBufferObject));

    DescriptorBuilder::begin(&Renderer::getAPI()->descriptorLayoutCache,
                             &Renderer::getAPI()->descriptorAllocator)
        .bind_buffer(0, &bufferInfo, vk::DescriptorType::eUniformBuffer,
                     vk::ShaderStageFlagBits::eVertex)
        .build(sets[i]);

    vk::DescriptorImageInfo imageInfo(
        textureSampler, Renderer::getTexture(material.diffuse).imageView,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    DescriptorBuilder::begin(&Renderer::getAPI()->descriptorLayoutCache,
                             &Renderer::getAPI()->descriptorAllocator)
        .bind_image(0, &imageInfo, vk::DescriptorType::eCombinedImageSampler,
                    vk::ShaderStageFlagBits::eFragment)
        .build(material.sets[i]);
  }
}

void VulkanAPI::createCommandPools() {
  ASH_INFO("Creating command pool");
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

  vk::CommandPoolCreateInfo poolInfo({},
                                     queueFamilyIndices.graphicsFamily.value());

  commandPool = device.createCommandPool(poolInfo);

  poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;

  transferCommandPool = device.createCommandPool(poolInfo);
}

uint32_t VulkanAPI::findMemoryType(uint32_t typeFilter,
                                   vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memProperties =
      physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;
  }

  ASH_ASSERT(false, "Failed to find suitable memory type");
  return -1;
}

void VulkanAPI::createCommandBuffers() {
  ASH_INFO("Creating command buffers");

  commandBuffers.resize(swapchainFramebuffers.size());

  vk::CommandBufferAllocateInfo allocInfo(
      commandPool, vk::CommandBufferLevel::ePrimary, commandBuffers.size());

  commandBuffers = device.allocateCommandBuffers(allocInfo);

  recordCommandBuffers();
}

void VulkanAPI::recordCommandBuffers() {
  for (size_t i = 0; i < commandBuffers.size(); i++) {
    commandBuffers[i].begin(vk::CommandBufferBeginInfo());

    vk::RenderPassBeginInfo renderPassInfo(renderPass, swapchainFramebuffers[i],
                                           {{0, 0}, swapchainExtent});

    std::array<vk::ClearValue, 2> clearValues{
        vk::ClearValue(
            {clearColor.r, clearColor.g, clearColor.b, clearColor.a}),
        vk::ClearValue({1, 0})};

    renderPassInfo.setClearValues(clearValues);

    commandBuffers[i].beginRenderPass(renderPassInfo,
                                      vk::SubpassContents::eInline);

    vk::Viewport viewport(0, 0, swapchainExtent.width, swapchainExtent.height,
                          0, 1);

    vk::Rect2D scissor({0, 0}, swapchainExtent);

    commandBuffers[i].setViewport(0, viewport);
    commandBuffers[i].setScissor(0, scissor);
    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                         pipelineLayout, 0,
                                         globalDescriptorSets[i], {});

    vk::DeviceSize offsets[] = {0};

    std::shared_ptr<Scene> scene = Renderer::getScene();
    if (scene) {
      auto renderables = scene->registry.view<Renderable>();

      for (auto entity : renderables) {
        auto &renderable = renderables.get(entity);

        Model &model = Renderer::getModel(renderable.model);
        for (uint32_t j = 0; j < model.meshes.size(); j++) {
          Mesh &mesh = Renderer::getMesh(model.meshes[j]);
          vk::Buffer vb[] = {mesh.ivb.buffer};

          commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                               pipelineLayout, 1,
                                               model.materials[j].sets[i], {});

          // Each model should have their own pipeline
          commandBuffers[i].bindPipeline(
              vk::PipelineBindPoint::eGraphics,
              graphicsPipelines[renderable.pipeline]);

          // Each model has their own mesh and thus their own vertex
          // and index buffers
          commandBuffers[i].bindVertexBuffers(0, vb, offsets);
          commandBuffers[i].bindIndexBuffer(mesh.ivb.buffer, mesh.ivb.vertSize,
                                            vk::IndexType::eUint32);

          // Each entity has their own transform and thus their own
          // UBO transform matrix
          commandBuffers[i].bindDescriptorSets(
              vk::PipelineBindPoint::eGraphics, pipelineLayout, 2,
              renderable.descriptorSets[j][i], {});

          commandBuffers[i].drawIndexed(mesh.ivb.numIndices, 1, 0, 0, 0);
        }
      }
    }

    commandBuffers[i].endRenderPass();

    commandBuffers[i].end();
  }
}

void VulkanAPI::createSyncObjects() {
  ASH_INFO("Creating synchronization objects");

  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    imageAvailableSemaphores[i] = device.createSemaphore({});
    renderFinishedSemaphores[i] = device.createSemaphore({});
    inFlightFences[i] =
        device.createFence({vk::FenceCreateFlagBits::eSignaled});
  }

  copyFinishedFence = device.createFence({vk::FenceCreateFlagBits::eSignaled});
}

void VulkanAPI::cleanupSwapchain() {
  device.destroyImageView(depthImageView);

  vmaDestroyImage(allocator, depthImage, depthImageAllocation);

  for (auto framebuffer : swapchainFramebuffers)
    device.destroyFramebuffer(framebuffer);

  device.freeCommandBuffers(commandPool, commandBuffers);

  device.destroyRenderPass(renderPass);

  for (auto imageView : swapchainImageViews)
    device.destroyImageView(imageView);

  device.destroySwapchainKHR(swapchain);
}

void VulkanAPI::recreateSwapchain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(App::getWindow()->get(), &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(App::getWindow()->get(), &width, &height);
    glfwWaitEvents();
  }

  device.waitIdle();

  ASH_INFO("Recreating swapchain");

  cleanupSwapchain();

  createSwapchain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createCommandBuffers();
}

void VulkanAPI::createBuffer(vk::DeviceSize size, VmaMemoryUsage memUsage,
                             vk::BufferUsageFlags usage, vk::Buffer &buffer,
                             VmaAllocation &allocation,
                             VmaAllocationCreateFlags flags) {
  vk::BufferCreateInfo bufferInfo({}, size, usage);

  VmaAllocationCreateInfo allocationInfo{};
  allocationInfo.usage = memUsage;
  allocationInfo.flags = flags;

  ASH_ASSERT(vmaCreateBuffer(
                 allocator, reinterpret_cast<VkBufferCreateInfo *>(&bufferInfo),
                 &allocationInfo, reinterpret_cast<VkBuffer *>(&buffer),
                 &allocation, nullptr) == VK_SUCCESS,
             "Failed to create buffer and allocation");
}

void VulkanAPI::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer,
                           vk::DeviceSize size) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::BufferCopy copyRegion{};
  copyRegion.size = size;
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

  endSingleTimeCommands(commandBuffer);
}

void VulkanAPI::copyBufferToImage(vk::Buffer buffer, vk::Image image,
                                  uint32_t width, uint32_t height) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::BufferImageCopy region;
  region.setImageSubresource(
      vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
  region.setImageOffset(vk::Offset3D(0, 0, 0));
  region.setImageExtent(vk::Extent3D(width, height, 1));

  commandBuffer.copyBufferToImage(buffer, image,
                                  vk::ImageLayout::eTransferDstOptimal, region);

  endSingleTimeCommands(commandBuffer);
}

vk::ShaderModule VulkanAPI::createShaderModule(const std::vector<char> &code) {
  vk::ShaderModuleCreateInfo createInfo(
      {}, code.size(), reinterpret_cast<const uint32_t *>(code.data()));

  return device.createShaderModule(createInfo);
}

void VulkanAPI::createImage(uint32_t width, uint32_t height,
                            VmaMemoryUsage memUsage, vk::Format format,
                            vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                            vk::Image &image, VmaAllocation &allocation) {
  vk::ImageCreateInfo imageInfo(
      {}, vk::ImageType::e2D, format, vk::Extent3D(width, height, 1), 1, 1,
      vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive);

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = memUsage;

  ASH_ASSERT(vmaCreateImage(
                 allocator, reinterpret_cast<VkImageCreateInfo *>(&imageInfo),
                 &allocCreateInfo, reinterpret_cast<VkImage *>(&image),
                 &allocation, nullptr) == VK_SUCCESS,
             "Failed to create device image");
}

void VulkanAPI::transitionImageLayout(vk::Image image, vk::Format format,
                                      vk::ImageLayout oldLayout,
                                      vk::ImageLayout newLayout) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
  barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
  barrier.image = image;

  if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

    if (hasStencilComponent(format))
      barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
  } else {
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  }

  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = {}; // TODO
  barrier.dstAccessMask = {}; // TODO

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined &&
      newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else if (oldLayout == vk::ImageLayout::eUndefined &&
             newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
                            vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
  } else {
    ASH_ASSERT(false, "Unsupported image layout transition");
    return;
  }

  commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {},
                                barrier);

  endSingleTimeCommands(commandBuffer);
}

void VulkanAPI::createTextureImage(const std::string &path, Texture &texture) {
  ASH_INFO("Loading texture {}", path);

  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels,
                              STBI_rgb_alpha);

  vk::DeviceSize imageSize = texWidth * texHeight * 4;

  ASH_ASSERT(pixels, "Failed to load image from disk");

  vk::Buffer stagingBuffer;
  VmaAllocation stagingBufferAllocation;

  createBuffer(imageSize, VMA_MEMORY_USAGE_AUTO,
               vk::BufferUsageFlagBits::eTransferSrc, stagingBuffer,
               stagingBufferAllocation,
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

  void *data;
  vmaMapMemory(allocator, stagingBufferAllocation, &data);
  std::memcpy(data, pixels, static_cast<size_t>(imageSize));
  vmaUnmapMemory(allocator, stagingBufferAllocation);

  stbi_image_free(pixels);

  createImage(texWidth, texHeight, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
              vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
              vk::ImageUsageFlagBits::eTransferDst |
                  vk::ImageUsageFlagBits::eSampled,
              texture.image, texture.imageAllocation);

  transitionImageLayout(texture.image, vk::Format::eR8G8B8A8Srgb,
                        vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eTransferDstOptimal);
  copyBufferToImage(stagingBuffer, texture.image,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  transitionImageLayout(texture.image, vk::Format::eR8G8B8A8Srgb,
                        vk::ImageLayout::eTransferDstOptimal,
                        vk::ImageLayout::eShaderReadOnlyOptimal);

  vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);

  createTextureImageView(texture);

  textures.push_back(texture);
}

vk::ImageView VulkanAPI::createImageView(vk::Image image, vk::Format format,
                                         vk::ImageAspectFlags aspectFlags) {
  vk::ImageViewCreateInfo viewInfo(
      {}, image, vk::ImageViewType::e2D, format, {},
      vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1));

  return device.createImageView(viewInfo);
}

void VulkanAPI::createTextureImageView(Texture &texture) {
  texture.imageView = createImageView(texture.image, vk::Format::eR8G8B8A8Srgb,
                                      vk::ImageAspectFlagBits::eColor);
}

void VulkanAPI::createTextureSampler() {
  vk::SamplerCreateInfo samplerInfo({}, vk::Filter::eLinear,
                                    vk::Filter::eLinear);
  samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
  samplerInfo.anisotropyEnable = VK_TRUE;

  vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

  samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
  samplerInfo.unnormalizedCoordinates = vk::False;
  samplerInfo.compareEnable = vk::False;
  samplerInfo.compareOp = vk::CompareOp::eAlways;
  samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

  textureSampler = device.createSampler(samplerInfo);
}

void VulkanAPI::createSurface() {
  GLFWwindow *window = Ash::App::getWindow()->get();
  VkResult result = glfwCreateWindowSurface(
      instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
  ASH_ASSERT(result == VK_SUCCESS, "Failed to create window surface, {}",
             result);

  ASH_INFO("Created Vulkan surface");
}

void VulkanAPI::init(const std::vector<Pipeline> &pipelines) {
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
  createUniformBuffers(globalUniformBuffers, sizeof(GlobalBufferObject));
  createUniformBuffers(globalLightUniformBuffers, sizeof(LightBufferObject));
  createDescriptorSetLayouts();
  createGraphicsPipelines(pipelines);
  createDescriptorAllocator();
  createGlobalDescriptorSets();
  createCommandPools();
  createDepthResources();
  createFramebuffers();
  createCommandBuffers();
  createTextureSampler();
  createSyncObjects();
}

void VulkanAPI::updateUniformBuffers(uint32_t currentImage) {
  GlobalBufferObject gbo{};
  gbo.view = Renderer::getCamera().getView();
  gbo.proj =
      glm::perspective(glm::radians(Renderer::getCamera().fov),
                       swapchainExtent.width / (float)swapchainExtent.height,
                       Renderer::getCamera().near, Renderer::getCamera().far);
  gbo.proj[1][1] *= -1;

  void *data;
  vmaMapMemory(allocator,
               globalUniformBuffers[currentImage].uniformBufferAllocation,
               &data);
  std::memcpy(data, &gbo, sizeof(gbo));
  vmaUnmapMemory(allocator,
                 globalUniformBuffers[currentImage].uniformBufferAllocation);

  LightBufferObject lbo{glm::vec4(1.0, 5.0, 0.0, 1.0),
                        glm::vec4(1.0, 1.0, 1.0, 1.0)};
  
  vmaMapMemory(allocator,
               globalLightUniformBuffers[currentImage].uniformBufferAllocation,
               &data);
  std::memcpy(data, &lbo, sizeof(lbo));
  vmaUnmapMemory(allocator,
                 globalLightUniformBuffers[currentImage].uniformBufferAllocation);

  RenderableBufferObject ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

  std::shared_ptr<Scene> scene = Renderer::getScene();
  if (scene) {
    auto renderables = scene->registry.view<Renderable, Transform>();
    for (auto entity : renderables) {
      auto [renderable, transform] =
          renderables.get<Renderable, Transform>(entity);

      ubo.model = transform.getTransform();

      void *data;
      vmaMapMemory(allocator,
                   renderable.ubos[currentImage].uniformBufferAllocation,
                   &data);
      std::memcpy(data, &ubo, sizeof(ubo));
      vmaUnmapMemory(allocator,
                     renderable.ubos[currentImage].uniformBufferAllocation);
    }
  }
}

void VulkanAPI::render() {
  updateCommandBuffers();

  ASH_ASSERT(device.waitForFences(inFlightFences[currentFrame], vk::True,
                                  UINT64_MAX) == vk::Result::eSuccess,
             "Error while waiting for in-flight fence");

  auto [result, imageIndex] = device.acquireNextImageKHR(
      swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame]);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapchain();
    return;
  }

  ASH_ASSERT(result == vk::Result::eSuccess ||
                 result == vk::Result::eSuboptimalKHR,
             "Failed to acquire swapchain image");

  if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    ASH_ASSERT(device.waitForFences(imagesInFlight[imageIndex], vk::True,
                                    UINT64_MAX) == vk::Result::eSuccess,
               "Error while waiting for images-in-flight fence");

  imagesInFlight[imageIndex] = inFlightFences[currentFrame];

  updateUniformBuffers(imageIndex);

  vk::PipelineStageFlags waitStages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::SubmitInfo submitInfo(imageAvailableSemaphores[currentFrame], waitStages,
                            commandBuffers[imageIndex],
                            renderFinishedSemaphores[currentFrame]);

  device.resetFences(inFlightFences[currentFrame]);

  graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

  vk::PresentInfoKHR presentInfo(renderFinishedSemaphores[currentFrame],
                                 swapchain, imageIndex);

  try {
    result = presentQueue.presentKHR(presentInfo);
  } catch (std::exception const &e) {
    if (result == vk::Result::eErrorOutOfDateKHR ||
        result == vk::Result::eSuboptimalKHR ||
        App::getWindow()->framebufferResized) {
      App::getWindow()->framebufferResized = false;
      recreateSwapchain();
    }
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanAPI::cleanup() {
  device.waitIdle();

  ASH_INFO("Cleaning up graphics API");

  device.destroyPipelineCache(pipelineCache);

  cleanupSwapchain();

  device.destroySampler(textureSampler);

  for (auto texture : textures) {
    device.destroyImageView(texture.imageView);
    vmaDestroyImage(allocator, texture.image, texture.imageAllocation);
  }

  for (auto pipeline : graphicsPipelines)
    device.destroyPipeline(pipeline.second);

  device.destroyPipelineLayout(pipelineLayout);

  std::shared_ptr<Scene> scene = Renderer::getScene();
  if (scene) {
    auto renderables = scene->registry.view<Renderable>();
    for (auto entity : renderables) {
      auto &renderable = renderables.get<Renderable>(entity);
      for (auto buffer : renderable.ubos) {
        vmaDestroyBuffer(allocator, buffer.uniformBuffer,
                         buffer.uniformBufferAllocation);
      }
    }
  }

  for (auto buffer : globalUniformBuffers)
    vmaDestroyBuffer(allocator, buffer.uniformBuffer,
                     buffer.uniformBufferAllocation);

  for (auto buffer : globalLightUniformBuffers)
    vmaDestroyBuffer(allocator, buffer.uniformBuffer,
                     buffer.uniformBufferAllocation);

  descriptorLayoutCache.cleanup();
  descriptorAllocator.cleanup();

  for (IndexedVertexBuffer ivb : indexedVertexBuffers) {
    vmaDestroyBuffer(allocator, ivb.buffer, ivb.bufferAllocation);
  }

  vmaDestroyAllocator(allocator);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    device.destroySemaphore(imageAvailableSemaphores[i]);
    device.destroySemaphore(renderFinishedSemaphores[i]);
    device.destroyFence(inFlightFences[i]);
  }

  device.destroyFence(copyFinishedFence);

  device.destroyCommandPool(commandPool);
  device.destroyCommandPool(transferCommandPool);

  device.destroy();

  if (enableValidationLayers) {
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }

  instance.destroySurfaceKHR(surface);
  instance.destroy();
}

void VulkanAPI::updateCommandBuffers() {
  graphicsQueue.waitIdle();
  device.resetCommandPool(commandPool);
  recordCommandBuffers();
}

vk::Format
VulkanAPI::findSupportedFormat(const std::vector<vk::Format> &candidates,
                               vk::ImageTiling tiling,
                               vk::FormatFeatureFlags features) {
  for (vk::Format format : candidates) {
    vk::FormatProperties props;
    props = physicalDevice.getFormatProperties(format);

    if (tiling == vk::ImageTiling::eLinear &&
        (props.linearTilingFeatures & features) == features)
      return format;
    else if (tiling == vk::ImageTiling::eOptimal &&
             (props.optimalTilingFeatures & features) == features)
      return format;
  }

  ASH_ASSERT(false, "Failed to find supported format");
}

bool VulkanAPI::hasStencilComponent(vk::Format format) {
  return format == vk::Format::eD32SfloatS8Uint ||
         format == vk::Format::eD24UnormS8Uint;
}

vk::Format VulkanAPI::findDepthFormat() {
  return findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
       vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

void VulkanAPI::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();

  createImage(swapchainExtent.width, swapchainExtent.height,
              VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, depthFormat,
              vk::ImageTiling::eOptimal,
              vk::ImageUsageFlagBits::eDepthStencilAttachment, depthImage,
              depthImageAllocation);
  depthImageView =
      createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);

  transitionImageLayout(depthImage, depthFormat, vk::ImageLayout::eUndefined,
                        vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

/*
 *
 *      Renderer API
 *
 */

void VulkanAPI::setClearColor(const glm::vec4 &color) { clearColor = color; }

IndexedVertexBuffer
VulkanAPI::createIndexedVertexArray(const std::vector<Vertex> &verts,
                                    const std::vector<uint32_t> &indices) {
  IndexedVertexBuffer ret{};
  ret.numIndices = indices.size();

  vk::DeviceSize vertSize = sizeof(verts[0]) * verts.size();
  ret.vertSize = vertSize;

  vk::DeviceSize indicesSize = sizeof(indices[0]) * indices.size();
  vk::DeviceSize bufferSize = vertSize + indicesSize;

  vk::Buffer stagingBuffer;
  VmaAllocation stagingBufferAllocation;
  createBuffer(bufferSize, VMA_MEMORY_USAGE_AUTO,
               vk::BufferUsageFlagBits::eTransferSrc, stagingBuffer,
               stagingBufferAllocation,
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

  void *data;
  vmaMapMemory(allocator, stagingBufferAllocation, &data);
  std::memcpy(data, verts.data(), static_cast<size_t>(vertSize));
  std::memcpy(static_cast<Vertex *>(data) + verts.size(), indices.data(),
              static_cast<size_t>(indicesSize));
  vmaUnmapMemory(allocator, stagingBufferAllocation);

  createBuffer(bufferSize, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
               vk::BufferUsageFlagBits::eTransferDst |
                   vk::BufferUsageFlagBits::eVertexBuffer |
                   vk::BufferUsageFlagBits::eIndexBuffer,
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
  std::vector<vk::LayerProperties> availableLayers =
      vk::enumerateInstanceLayerProperties();

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
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

} // namespace Ash
