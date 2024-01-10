#include "Descriptor.h"

#include "Core.h"

namespace Ash {

vk::DescriptorPool createPool(vk::Device device,
                              const DescriptorAllocator::PoolSizes &poolSizes,
                              int count, vk::DescriptorPoolCreateFlags flags) {
  std::vector<vk::DescriptorPoolSize> sizes;
  sizes.reserve(poolSizes.sizes.size());
  for (auto sz : poolSizes.sizes) {
    sizes.push_back({sz.first, uint32_t(sz.second * count)});
  }
  vk::DescriptorPoolCreateInfo pool_info(flags, count, sizes);

  vk::DescriptorPool descriptorPool;
  auto result =
      device.createDescriptorPool(&pool_info, nullptr, &descriptorPool);

  ASH_ASSERT(result == vk::Result::eSuccess,
             "Failed to create descriptor pool");

  return descriptorPool;
}

void DescriptorAllocator::init(vk::Device newDevice) { device = newDevice; }

void DescriptorAllocator::cleanup() {
  for (auto p : freePools) {
    device.destroyDescriptorPool(p, nullptr);
  }
  for (auto p : usedPools) {
    device.destroyDescriptorPool(p, nullptr);
  }
}

vk::DescriptorPool DescriptorAllocator::grab_pool() {
  // there are reusable pools availible
  if (freePools.size() > 0) {
    // grab pool from the back of the vector and remove it from there.
    vk::DescriptorPool pool = freePools.back();
    freePools.pop_back();
    return pool;
  } else {
    // no pools availible, so create a new one
    return createPool(device, descriptorSizes, 1000, {});
  }
}

bool DescriptorAllocator::allocate(vk::DescriptorSet &set,
                                   vk::DescriptorSetLayout layout) {
  // initialize the currentPool handle if it's null
  if (currentPool == VK_NULL_HANDLE) {
    currentPool = grab_pool();
    usedPools.push_back(currentPool);
  }

  // try to allocate the descriptor set
  bool needReallocate = false;
  try {
    set = device
              .allocateDescriptorSets(
                  vk::DescriptorSetAllocateInfo(currentPool, layout))
              .front();
  } catch (vk::SystemError &err) {
    ASH_WARN("Descriptor set allocation failed, need reallocate: {}",
             err.what());
    needReallocate = true;
  }

  if (needReallocate) {
    // allocate a new pool and retry
    currentPool = grab_pool();
    usedPools.push_back(currentPool);
    try {
      set = device
                .allocateDescriptorSets(
                    vk::DescriptorSetAllocateInfo(currentPool, layout))
                .front();
    } catch (vk::SystemError &err) {
      ASH_ERROR("Descriptor set allocation failed again: {}", err.what());
      return false;
    }
  }

  return true;
}

void DescriptorAllocator::reset_pools() {
  // reset all used pools and add them to the free pools
  for (auto p : usedPools) {
    device.resetDescriptorPool(p, {});
    freePools.push_back(p);
  }

  // clear the used pools, since we've put them all in the free pools
  usedPools.clear();

  // reset the current pool handle back to null
  currentPool = VK_NULL_HANDLE;
}

void DescriptorLayoutCache::init(vk::Device newDevice) { device = newDevice; }

void DescriptorLayoutCache::cleanup() {
  // delete every descriptor layout held
  for (auto pair : layoutCache) {
    device.destroyDescriptorSetLayout(pair.second, nullptr);
  }
}

vk::DescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(
    vk::DescriptorSetLayoutCreateInfo &info) {
  DescriptorLayoutInfo layoutinfo;
  layoutinfo.bindings.reserve(info.bindingCount);
  bool isSorted = true;
  int lastBinding = -1;

  // copy from the direct info struct into our own one
  for (uint32_t i = 0; i < info.bindingCount; i++) {
    layoutinfo.bindings.push_back(info.pBindings[i]);

    // check that the bindings are in strict increasing order
    if (static_cast<int>(info.pBindings[i].binding) > lastBinding) {
      lastBinding = info.pBindings[i].binding;
    } else {
      isSorted = false;
    }
  }
  // sort the bindings if they aren't in order
  if (!isSorted) {
    std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(),
              [](vk::DescriptorSetLayoutBinding &a,
                 vk::DescriptorSetLayoutBinding &b) {
                return a.binding < b.binding;
              });
  }

  // try to grab from cache
  auto it = layoutCache.find(layoutinfo);
  if (it != layoutCache.end()) {
    return (*it).second;
  } else {
    // create a new one (not found)
    vk::DescriptorSetLayout layout = device.createDescriptorSetLayout(info);

    // add to cache
    layoutCache[layoutinfo] = layout;
    return layout;
  }
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(
    const DescriptorLayoutInfo &other) const {
  if (other.bindings.size() != bindings.size()) {
    return false;
  } else {
    // compare each of the bindings is the same. Bindings are sorted so they
    // will match
    for (uint32_t i = 0; i < bindings.size(); i++) {
      if (other.bindings[i] != bindings[i])
        return false;
    }
    return true;
  }
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
  using std::hash;
  using std::size_t;

  size_t result = hash<size_t>()(bindings.size());

  for (const vk::DescriptorSetLayoutBinding &b : bindings) {
    // pack the binding data into a single int64. Not fully correct but it's ok
    size_t binding_hash = b.binding |
                          static_cast<VkDescriptorType>(b.descriptorType) << 8 |
                          b.descriptorCount << 16 |
                          static_cast<VkShaderStageFlags>(b.stageFlags) << 24;

    // shuffle the packed binding data and xor it with the main hash
    result ^= hash<size_t>()(binding_hash);
  }

  return result;
}

DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache *layoutCache,
                                           DescriptorAllocator *allocator) {
  DescriptorBuilder builder;
  builder.cache = layoutCache;
  builder.alloc = allocator;
  return builder;
}

DescriptorBuilder &DescriptorBuilder::bind_buffer(
    uint32_t binding, const vk::DescriptorBufferInfo *bufferInfo,
    vk::DescriptorType type, vk::ShaderStageFlags stageFlags) {
  // create the descriptor binding for the layout
  vk::DescriptorSetLayoutBinding newBinding(binding, type, 1, stageFlags);
  bindings.push_back(newBinding);

  // create the descriptor write
  vk::WriteDescriptorSet newWrite({}, binding, 0, 1, type, {}, bufferInfo);
  writes.push_back(newWrite);
  return *this;
}

DescriptorBuilder &DescriptorBuilder::bind_image(
    uint32_t binding, const vk::DescriptorImageInfo *imageInfo,
    vk::DescriptorType type, vk::ShaderStageFlags stageFlags) {
  // create the descriptor binding for the layout
  vk::DescriptorSetLayoutBinding newBinding(binding, type, 1, stageFlags);
  bindings.push_back(newBinding);

  // create the descriptor write
  vk::WriteDescriptorSet newWrite({}, binding, 0, 1, type, imageInfo, {});
  writes.push_back(newWrite);
  return *this;
}

bool DescriptorBuilder::build(vk::DescriptorSet &set,
                              vk::DescriptorSetLayout &layout) {
  // build layout first
  vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings);
  layout = cache->create_descriptor_layout(layoutInfo);

  // allocate descriptor
  bool success = alloc->allocate(set, layout);
  if (!success) {
    return false;
  };

  // write descriptor
  for (vk::WriteDescriptorSet &w : writes) {
    w.dstSet = set;
  }

  alloc->device.updateDescriptorSets(writes, {});

  return true;
}

bool DescriptorBuilder::build(vk::DescriptorSet &set) {
  vk::DescriptorSetLayout layout;
  return build(set, layout);
}

} // namespace Ash
