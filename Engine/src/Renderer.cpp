#include "Renderer.h"

namespace Ash {

std::unique_ptr<VulkanAPI> Renderer::api = std::make_unique<VulkanAPI>();

void Renderer::init() { api->init(); }

void Renderer::render() { api->render(); }

void Renderer::cleanup() { api->cleanup(); }

void Renderer::setClearColor(const glm::vec4& clearColor) {
    api->setClearColor(clearColor);
}

}  // namespace Ash
