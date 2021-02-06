#include "Renderer.h"

namespace Ash {

std::shared_ptr<VulkanAPI> Renderer::api = std::make_shared<VulkanAPI>();
std::vector<Pipeline> Renderer::pipelines;
std::unordered_map<std::string, Mesh> Renderer::meshes;
std::shared_ptr<Scene> Renderer::scene;

void Renderer::loadPipeline(const Pipeline& pipeline) {
    pipelines.push_back(pipeline);
}

void Renderer::setPipeline(const std::string& name) { api->setPipeline(name); }

void Renderer::loadMesh(const std::string& name,
                        const std::vector<Vertex>& verts,
                        const std::vector<uint32_t>& indices) {
    meshes[name] = {name, createIndexedVertexBuffer(verts, indices)};
}

void Renderer::init() { api->init(pipelines); }

void Renderer::render() { api->render(); }

void Renderer::cleanup() { api->cleanup(); }

IndexedVertexBuffer Renderer::createIndexedVertexBuffer(
    const std::vector<Vertex>& verts, const std::vector<uint32_t>& indices) {
    return api->createIndexedVertexArray(verts, indices);
}

void Renderer::setClearColor(const glm::vec4& clearColor) {
    api->setClearColor(clearColor);
}

void Renderer::setScene(std::shared_ptr<Scene> scene) {
    Renderer::scene = scene;
    api->signalRecord();
}

}  // namespace Ash
