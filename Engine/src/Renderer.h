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
    static void setPipeline(std::string name);

    static void init();
    static void render();
    static void cleanup();

    static void setClearColor(const glm::vec4& clearColor);
    static void drawVertices(std::vector<Vertex> verts);
    static void drawIndexedVertices(std::vector<Vertex> verts,
                                    std::vector<uint32_t> indices);

    static inline std::shared_ptr<VulkanAPI> getAPI() { return api; }

   private:
    static std::shared_ptr<VulkanAPI> api;
    static std::vector<Pipeline> pipelines;
};

}  // namespace Ash
