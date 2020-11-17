#include "VulkanAPI.h"

#include <GLFW/glfw3.h>

namespace Ash {

VulkanAPI::VulkanAPI() {}

VulkanAPI::~VulkanAPI() {}

void VulkanAPI::init() {
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
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        ASH_ERROR("Failed to initialize vulkan");
        throw std::runtime_error("");
    }

    uint32_t extensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount,
                                           extensions.data());

    ASH_INFO("Available extensions: ");
    for (const auto& extension : extensions) {
        ASH_INFO(extension.extensionName);
    }
}

void VulkanAPI::cleanup() {
    ASH_INFO("Destroying Vulkan instance...");
    vkDestroyInstance(instance, nullptr);
}

}  // namespace Ash
