#pragma once

#include <glm/glm.hpp>

#include <string>

#include "Pipeline.h"
#include "VulkanAPI.h"

namespace Ash {
class Renderer {
   public:
    Renderer();
    ~Renderer();

    static void loadPipeline(const Pipeline& pipeline);
    static void setPipeline(size_t i);

    static void init();
    static void render();
    static void cleanup();

    static void setClearColor(const glm::vec4& clearColor);

    static inline std::shared_ptr<VulkanAPI> getAPI() { return api; }

   private:
    static std::shared_ptr<VulkanAPI> api;
    static std::vector<Pipeline> pipelines;
};
}  // namespace Ash
