#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string>

#include "Helper.h"
#include "Pipeline.h"
#include "Scene.h"
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

    static Mesh& getMesh(const std::string& name) { return batch[name]; }

    // change to queueEntity and dequeueEntity
    // OR BETTER just render all entities with Renderable component and whether
    // they are visible or not by adding them appropriately to the batch if
    // they're supposed to be rendered
    static void queueMesh(const std::string& name);
    static void dequeueMesh(const std::string& name);

    static void init();
    static void render();
    static void cleanup();

    static void setClearColor(const glm::vec4& clearColor);
    static void setScene(std::shared_ptr<Scene> scene);

    static inline std::shared_ptr<VulkanAPI> getAPI() { return api; }

   private:
    static std::shared_ptr<VulkanAPI> api;

    static std::vector<Pipeline> pipelines;

    static std::shared_ptr<Scene> scene;

    // TODO: Change to <std::string (mesh names), std::vector<Entity>> for
    // better batch rendering
    static std::unordered_map<std::string, Mesh> batch;

    static std::unordered_map<std::string, Mesh> meshes;
};

}  // namespace Ash
