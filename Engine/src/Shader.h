#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

namespace Ash {

class Shader {
   public:
    Shader(const std::string& path, VkShaderStageFlagBits stage);
    ~Shader();

   private:
    VkShaderStageFlagBits stage;
};

}  // namespace Ash
