#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <array>
#include <vector>

#include "ashpch.h"

namespace Ash {
struct UniformBuffer {
    VkBuffer uniformBuffer;
    VmaAllocation uniformBufferAllocation;
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescription() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct IndexedVertexBuffer {
    uint32_t numIndices;
    VkDeviceSize vertSize;

    VkBuffer buffer;
    VmaAllocation bufferAllocation;
};

struct Mesh {
    std::string name;

    IndexedVertexBuffer ivb;
};

struct Texture {
    std::string name;

    VkImage image;
    VmaAllocation imageAllocation;
    VkImageView imageView;
};

struct Model {
    std::string name;

    std::vector<std::string> meshes;
    std::vector<std::string> textures;
};

namespace Helper {

std::vector<char> readBinaryFile(const char* filename);
bool importModel(const std::string& name, const std::string& file);

}  // namespace Helper

}  // namespace Ash
