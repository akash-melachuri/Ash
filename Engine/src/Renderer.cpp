#include "Renderer.h"

namespace Ash {

std::shared_ptr<VulkanAPI> Renderer::api = std::make_shared<VulkanAPI>();
std::vector<Pipeline> Renderer::pipelines;

void Renderer::loadPipeline(const Pipeline& pipeline) {
    pipelines.push_back(pipeline);
}

void Renderer::setPipeline(std::string name) { api->setPipeline(name); }

void Renderer::init() { api->init(pipelines); }

void Renderer::render() { api->render(); }

void Renderer::cleanup() { api->cleanup(); }

void Renderer::drawVertices(std::vector<Vertex> verts) {
    std::vector<uint32_t> indices(verts.size());

    for (uint32_t i = 0; i < verts.size(); i++) {
        indices[i] = i;
    }

    api->submitIndexedVertexArray(verts, indices);
}

void Renderer::drawIndexedVertices(std::vector<Vertex> verts,
                                   std::vector<uint32_t> indices) {
    api->submitIndexedVertexArray(verts, indices);
}

void Renderer::setClearColor(const glm::vec4& clearColor) {
    api->setClearColor(clearColor);
}

}  // namespace Ash
