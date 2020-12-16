#pragma once

#include <glm/glm.hpp>

#include "VulkanAPI.h"

namespace Ash {
class Renderer {
   public:
    Renderer();
    ~Renderer();

    static void init();
    static void cleanup();

    static void setClearColor(const glm::vec4& clearColor);

   private:
    static std::unique_ptr<VulkanAPI> api;
};
}  // namespace Ash
