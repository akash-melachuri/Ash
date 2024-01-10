#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

namespace Ash {

class DescriptorAllocator {
public:
  struct PoolSizes {
    std::vector<std::pair<vk::DescriptorType, float>> sizes = {
        {vk::DescriptorType::eSampler, 0.5f},
        {vk::DescriptorType::eCombinedImageSampler, 4.f},
        {vk::DescriptorType::eSampledImage, 4.f},
        {vk::DescriptorType::eStorageImage, 0.5f},
        {vk::DescriptorType::eUniformTexelBuffer, 0.5f},
        {vk::DescriptorType::eStorageTexelBuffer, 0.5f},
        {vk::DescriptorType::eUniformBuffer, 2.f},
        {vk::DescriptorType::eStorageBuffer, 0.5f},
        {vk::DescriptorType::eUniformBufferDynamic, 1.f},
        {vk::DescriptorType::eStorageBufferDynamic, 0.5f},
        {vk::DescriptorType::eInputAttachment, 0.5f}};
  };

  void reset_pools();
  bool allocate(vk::DescriptorSet &set, vk::DescriptorSetLayout layout);

  void init(vk::Device newDevice);

  void cleanup();

  vk::Device device;

private:
  vk::DescriptorPool grab_pool();

  vk::DescriptorPool currentPool{VK_NULL_HANDLE};
  PoolSizes descriptorSizes;
  std::vector<vk::DescriptorPool> usedPools;
  std::vector<vk::DescriptorPool> freePools;
};

class DescriptorLayoutCache {
public:
  void init(vk::Device newDevice);
  void cleanup();

  vk::DescriptorSetLayout
  create_descriptor_layout(vk::DescriptorSetLayoutCreateInfo &info);

  struct DescriptorLayoutInfo {
    // good idea to turn this into a inlined array
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    bool operator==(const DescriptorLayoutInfo &other) const;

    size_t hash() const;
  };

private:
  struct DescriptorLayoutHash {

    std::size_t operator()(const DescriptorLayoutInfo &k) const {
      return k.hash();
    }
  };

  std::unordered_map<DescriptorLayoutInfo, vk::DescriptorSetLayout,
                     DescriptorLayoutHash>
      layoutCache;
  vk::Device device;
};

class DescriptorBuilder {
public:
  static DescriptorBuilder begin(DescriptorLayoutCache *layoutCache,
                                 DescriptorAllocator *allocator);

  DescriptorBuilder &bind_buffer(uint32_t binding,
                                 const vk::DescriptorBufferInfo *bufferInfo,
                                 vk::DescriptorType type,
                                 vk::ShaderStageFlags stageFlags);
  DescriptorBuilder &bind_image(uint32_t binding,
                                const vk::DescriptorImageInfo *imageInfo,
                                vk::DescriptorType type,
                                vk::ShaderStageFlags stageFlags);

  bool build(vk::DescriptorSet &set, vk::DescriptorSetLayout &layout);
  bool build(vk::DescriptorSet &set);

private:
  std::vector<vk::WriteDescriptorSet> writes;
  std::vector<vk::DescriptorSetLayoutBinding> bindings;

  DescriptorLayoutCache *cache;
  DescriptorAllocator *alloc;
};

} // namespace Ash
