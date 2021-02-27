#include "Helper.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <fstream>

#include "Core.h"
#include "Renderer.h"

namespace Ash::Helper {

std::vector<char> readBinaryFile(const char* filename) {
    std::ifstream istream(filename, std::ios::ate | std::ios::binary);

    ASH_ASSERT(istream.is_open(), "Failed to open file {}", filename);

    size_t size = (size_t)istream.tellg();
    std::vector<char> buffer(size);
    istream.seekg(0);
    istream.read(buffer.data(), size);
    istream.close();

    return buffer;
}

void processMesh(aiMesh* mesh, const aiScene* scene, const std::string& name,
                 uint32_t iteration) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(mesh->mNumVertices);

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.pos.x = mesh->mVertices[i].x;
        vertex.pos.y = mesh->mVertices[i].y;
        vertex.pos.z = mesh->mVertices[i].z;
        if (mesh->mTextureCoords[0]) {
            vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
        }
        vertices.push_back(vertex);
    }

    Renderer::loadMesh(name + "_" + std::to_string(iteration), vertices,
                       indices);
}

void processNode(aiNode* node, const aiScene* scene, const std::string& name,
                 uint32_t iteration) {
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, name, ++iteration);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, name, iteration);
    }

    std::vector<std::string> meshes;
    std::vector<std::string> textures;

    meshes.reserve(iteration);

    for (uint32_t i = 0; i < iteration; i++) {
        meshes.emplace_back(name + "_" + std::to_string(iteration));
    }

    Renderer::loadModel(name, meshes, textures);
}

bool importModel(const std::string& name, const std::string& file) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate);

    ASH_ASSERT(scene, "Failed to import mesh {}", file);

    if (!scene) {
        ASH_ERROR("Failed to import mesh {}", file);
        return false;
    }

    Helper::processNode(scene->mRootNode, scene, name, 0);

    return true;
}

}  // namespace Ash::Helper
