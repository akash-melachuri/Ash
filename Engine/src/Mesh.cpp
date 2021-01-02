#include "Mesh.h"

#include "Renderer.h"

namespace Ash {

Mesh::Mesh() {}
Mesh::Mesh(const std::string& name, const std::vector<Vertex>& verts,
           const std::vector<uint32_t>& indices) {
    ivb = Renderer::createIndexedVertexBuffer(verts, indices);
    this->name = name;
}

Mesh::~Mesh() {}

}  // namespace Ash
