#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "Helper.h"
#include "Renderer.h"

namespace Ash {

struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    Transform(const glm::vec3& pos) : position(pos){};

    glm::mat4 getTransform() const {
        glm::mat4 rotationMatrix = glm::toMat4(glm::quat(rotation));

        return glm::translate(glm::mat4(1.0f), position) * rotationMatrix *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

struct Renderable {
    std::vector<UniformBuffer> ubos;
    std::vector<VkDescriptorSet> descriptorSets;

    Renderable(const std::string& mesh, const std::string& pipeline)
        : mesh(mesh), pipeline(pipeline) {
        Renderer::getAPI()->createUniformBuffers(ubos);
        Renderer::getAPI()->createDescriptorSets(descriptorSets, ubos);
    }

    std::string mesh;
    std::string pipeline;
};

}  // namespace Ash
