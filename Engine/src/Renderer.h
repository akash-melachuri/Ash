#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string>

#include "Mesh.h"
#include "Pipeline.h"
#include "VulkanAPI.h"

namespace Ash {

class Renderer {
   public:
    Renderer();
    ~Renderer();

    static void loadPipeline(const Pipeline& pipeline);
    static void setPipeline(const std::string& name);

    static IndexedVertexBuffer createIndexedVertexBuffer(
        const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices);

    static void loadMesh(const std::string& name,
                         const std::vector<Vertex>& verts,
                         const std::vector<uint32_t>& indices);
    static void queueMesh(const std::string& name);
    static void dequeueMesh(const std::string& name);

    static Mesh& getMesh(const std::string& name) { return batch[name]; }

    static void init();
    static void render();
    static void cleanup();

    static void setClearColor(const glm::vec4& clearColor);

    static inline std::shared_ptr<VulkanAPI> getAPI() { return api; }

   private:
    static std::shared_ptr<VulkanAPI> api;
    static std::vector<Pipeline> pipelines;

    // TODO: Change from meshes to Entities that store Mesh names
    // VulkanAPI uses getMesh in order to retrieve the right buffer for each
    // entity
    static std::unordered_map<std::string, Mesh> batch;

    static std::unordered_map<std::string, Mesh> meshes;
};

}  // namespace Ash
