#pragma once

#include <string>

#include "Helper.h"

namespace Ash {

class Mesh {
   public:
    Mesh();
    Mesh(const std::string& name, const std::vector<Vertex>& verts,
         const std::vector<uint32_t>& indices);
    ~Mesh();

    std::string name;
    IndexedVertexBuffer ivb;
};

}  // namespace Ash
