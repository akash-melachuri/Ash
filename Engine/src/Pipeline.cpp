#include "Pipeline.h"

namespace Ash {

Pipeline::Pipeline(const std::string& vert, const std::string& frag,
                   const std::string& name) {
    this->name = name;

    paths.push_back(vert);
    paths.push_back(frag);

    stages.push_back(VERTEX_SHADER_STAGE);
    stages.push_back(FRAGMENT_SHADER_STAGE);
}

Pipeline::~Pipeline() {}

}  // namespace Ash
