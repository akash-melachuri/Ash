#include "Renderer.h"

namespace Ash {

std::shared_ptr<VulkanAPI> Renderer::api = std::make_shared<VulkanAPI>();
std::vector<Pipeline> Renderer::pipelines;
std::unordered_map<std::string, Mesh> Renderer::meshes;
std::shared_ptr<Scene> Renderer::scene;
std::unordered_map<std::string, Texture> Renderer::textures;
std::unordered_map<std::string, Model> Renderer::models;
Camera Renderer::camera;

void Renderer::loadModel(const std::string &name,
                         const std::vector<std::string> &meshes,
                         const std::vector<std::string> &textures) {
  models[name] = {name, meshes, textures};
}

void Renderer::loadPipeline(const Pipeline &pipeline) {
  pipelines.push_back(pipeline);
}

void Renderer::loadMesh(const std::string &name,
                        const std::vector<Vertex> &verts,
                        const std::vector<uint32_t> &indices) {
  meshes[name] = {name, api->createIndexedVertexArray(verts, indices)};
}

void Renderer::loadTexture(const std::string &name, const std::string &path) {
  if (textures.contains(name)) {
    ASH_WARN("Texture ID {} already exists, aborting texture loading", name);
    return;
  }
  api->createTextureImage(path, textures[name]);
}

void Renderer::init() {
  api->init(pipelines);
  loadTexture("white", "assets/textures/white.png");
}

void Renderer::render() { api->render(); }

void Renderer::cleanup() { api->cleanup(); }

void Renderer::setClearColor(const glm::vec4 &clearColor) {
  api->setClearColor(clearColor);
}

void Renderer::setScene(std::shared_ptr<Scene> scene) {
  Renderer::scene = scene;
}

void Renderer::setCamera(const Camera &camera) { Renderer::camera = camera; }

} // namespace Ash
