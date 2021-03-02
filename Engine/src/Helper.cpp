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

    size_t numIndices = 0;
    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        numIndices += face.mNumIndices;
    }

    indices.reserve(numIndices);
    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    Renderer::loadMesh(name + "_" + std::to_string(iteration), vertices,
                       indices);
}

std::vector<std::string> loadTextures(std::string name, aiMaterial* mat,
                                      aiTextureType type,
                                      std::string typeName) {
    std::vector<std::string> textures;
    textures.resize(mat->GetTextureCount(type));
    for (uint32_t i = 0; i < mat->GetTextureCount(type); i++) {
        aiString path;
        mat->GetTexture(type, i, &path);
        Renderer::loadTexture(name + typeName + std::to_string(i),
                              path.C_Str());
        textures.push_back(name + typeName + std::to_string(i));
        ASH_WARN("Path: {}", path.C_Str());
    }

    return textures;
}

void processNode(const aiScene* scene, const std::string& name,
                 uint32_t iteration) {
    std::vector<std::string> meshes;
    std::vector<std::string> textures;

    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        processMesh(mesh, scene, name, ++iteration);

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            std::vector<std::string> texs =
                loadTextures(name, material, aiTextureType_DIFFUSE, "diffuse");
            textures.insert(textures.end(), texs.begin(), texs.end());
        } else {
            textures.push_back("statue");
        }
    }

    meshes.reserve(iteration);

    for (uint32_t i = 0; i < iteration; i++) {
        meshes.emplace_back(name + "_" + std::to_string(i));
    }

    ASH_WARN("Textures: {}", textures.size());
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

    std::vector<std::string> meshes;
    std::vector<std::string> textures;
    Helper::processNode(scene, name, 0);

    return true;
}

}  // namespace Ash::Helper
