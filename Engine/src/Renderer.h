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

    static void loadMesh(const std::string& name,
                         const std::vector<Vertex>& verts,
                         const std::vector<uint32_t>& indices);
    // TODO Compare inline performance
    static Mesh& getMesh(const std::string& name) { return meshes[name]; }

    static void loadTexture(const std::string& name, const std::string& path);
    // TODO Compare inline performance
    static Texture& getTexture(const std::string& name) {
        return textures[name];
    }

    static void init();
    static void render();
    static void cleanup();

    static void setClearColor(const glm::vec4& clearColor);
    static void setScene(std::shared_ptr<Scene> scene);

    static inline std::shared_ptr<Scene> getScene() { return scene; }

    static inline std::shared_ptr<VulkanAPI> getAPI() { return api; }

   private:
    static std::shared_ptr<VulkanAPI> api;

    static std::vector<Pipeline> pipelines;

    static std::shared_ptr<Scene> scene;

    static std::unordered_map<std::string, Mesh> meshes;
    static std::unordered_map<std::string, Texture> textures;
};

}  // namespace Ash
