#pragma once

#include <string>
#include <vector>

namespace Ash {

enum ShaderStages { VERTEX_SHADER_STAGE, FRAGMENT_SHADER_STAGE };

class Pipeline {
   public:
    Pipeline(const std::string& vert, const std::string& frag,
             const std::string& name);
    ~Pipeline();

    std::vector<std::string> paths;
    std::vector<Ash::ShaderStages> stages;
    std::string name;
};

}  // namespace Ash
