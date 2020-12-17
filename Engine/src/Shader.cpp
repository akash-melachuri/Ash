#include "Shader.h"

namespace Ash {

Shader::Shader(const std::string& path, VkShaderStageFlagBits stage) {
    this->stage = stage;
}

Shader::~Shader() {}

}  // namespace Ash
