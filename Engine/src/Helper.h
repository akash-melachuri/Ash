#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <array>
#include <vector>

#include "ashpch.h"

namespace Ash {
struct UniformBuffer {
  vk::Buffer uniformBuffer;
  VmaAllocation uniformBufferAllocation;
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 texCoord;

  static vk::VertexInputBindingDescription getBindingDescription() {
    vk::VertexInputBindingDescription bindingDescription(
        0, sizeof(Vertex), vk::VertexInputRate::eVertex);
    return bindingDescription;
  }

  static std::array<vk::VertexInputAttributeDescription, 3>
  getAttributeDescription() {
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions;

    attributeDescriptions[0] = vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
    attributeDescriptions[1] = vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal));
    attributeDescriptions[2] = vk::VertexInputAttributeDescription(
        2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

    return attributeDescriptions;
  }
};

struct IndexedVertexBuffer {
  uint32_t numIndices;
  vk::DeviceSize vertSize;

  vk::Buffer buffer;
  VmaAllocation bufferAllocation;
};

struct Mesh {
  std::string name;

  IndexedVertexBuffer ivb;
};

struct Texture {
  std::string name;

  vk::Image image;
  VmaAllocation imageAllocation;
  vk::ImageView imageView;
};

struct Model {
  std::string name;

  std::vector<std::string> meshes;
  std::vector<std::string> textures;
};

namespace Helper {

std::vector<char> readBinaryFile(const char *filename);
bool importModel(const std::string &name, const std::string &file,
                 uint32_t flags = 0);

} // namespace Helper

} // namespace Ash
