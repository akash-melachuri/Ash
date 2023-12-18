#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace Ash {

class Camera {
public:
  inline glm::mat4 getView() { return glm::lookAt(eye, center, up); }

  glm::vec3 eye{2, 0, 0};
  glm::vec3 center{0};
  glm::vec3 up{0, 0, 1};

  float fov{45};
  float near{0.1};
  float far{10};
};

} // namespace Ash
