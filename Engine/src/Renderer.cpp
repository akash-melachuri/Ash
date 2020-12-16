#include "Renderer.h"

namespace Ash {

void Renderer::init() { api = std::make_unique<VulkanAPI>(); }

void Renderer::cleanup() { api->cleanup(); }

void Renderer::setClearColor(const glm::vec4& clearColor) {
    api->setClearColor(clearColor);
}

}  // namespace Ash
