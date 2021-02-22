#include "Helper.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include <fstream>

#include "Core.h"

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

void processMesh(aiMesh* mesh, const aiScene* scene) {}

void processNode(aiNode* node, const aiScene* scene) {
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

bool importModel(const std::string& file) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate);

    ASH_ASSERT(scene, "Failed to import mesh {}", file);

    if (!scene) {
        ASH_ERROR("Failed to import mesh {}", file);
        return false;
    }

    Helper::processNode(scene->mRootNode, scene);

    return true;
}

}  // namespace Ash::Helper
