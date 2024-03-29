#include "Helper.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>

#include "Core.h"
#include "Renderer.h"

namespace Ash::Helper {

std::vector<char> readBinaryFile(const char *filename) {
  std::ifstream istream(filename, std::ios::ate | std::ios::binary);

  ASH_ASSERT(istream.is_open(), "Failed to open file {}", filename);

  size_t size = (size_t)istream.tellg();
  std::vector<char> buffer(size);
  istream.seekg(0);
  istream.read(buffer.data(), size);
  istream.close();

  return buffer;
}

void processMesh(aiMesh *mesh, const std::string &name) {
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
    vertex.normal.x = mesh->mNormals[i].x;
    vertex.normal.y = mesh->mNormals[i].y;
    vertex.normal.z = mesh->mNormals[i].z;
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

  Renderer::loadMesh(name, vertices, indices);
}

std::vector<std::string> loadTextures(const std::string &directory,
                                      aiMaterial *mat, aiTextureType type) {
  std::vector<std::string> textures;
  for (uint32_t i = 0; i < mat->GetTextureCount(type); i++) {
    aiString path;
    mat->GetTexture(type, i, &path);
    Renderer::loadTexture(directory + std::string(path.C_Str()),
                          directory + std::string(path.C_Str()));
    textures.push_back(directory + std::string(path.C_Str()));
  }

  return textures;
}

void processNode(const aiNode *node, const aiScene *scene,
                 const std::string &name, const std::string &directory,
                 std::vector<std::string> &meshes,
                 std::vector<std::string> &diffuseTextures) {
  for (uint32_t i = 0; i < node->mNumMeshes; i++) {
    std::string mesh_name = name + "_" + std::to_string(node->mMeshes[i]);
    if (Renderer::hasMesh(mesh_name))
      continue;

    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    processMesh(mesh, mesh_name);
    meshes.push_back(mesh_name);
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<std::string> texs =
        loadTextures(directory, material, aiTextureType_DIFFUSE);
    diffuseTextures.insert(diffuseTextures.end(), texs.begin(), texs.end());

    if (texs.empty()) {
      ASH_INFO("Using backup texture");
      diffuseTextures.push_back("white");
    }
  }

  for (uint32_t i = 0; i < node->mNumChildren; i++)
    processNode(node->mChildren[i], scene, name, directory, meshes, diffuseTextures);
}

bool importModel(const std::string &name, const std::string &file,
                 uint32_t flags) {
  Assimp::Importer importer;

  const aiScene *scene = importer.ReadFile(
      file, flags | aiProcess_Triangulate | aiProcess_PreTransformVertices |
                aiProcess_OptimizeMeshes);

#ifdef ASH_WINDOWS
  char separator = '\\';
#else
  char separator = '/';
#endif

  std::string directory = file.substr(0, file.find_last_of(separator) + 1);

  ASH_ASSERT(scene, "Failed to import mesh {}", file);

  std::vector<std::string> meshes;
  std::vector<std::string> diffuseTextures;
  Helper::processNode(scene->mRootNode, scene, name, directory, meshes,
                      diffuseTextures);

  std::vector<Material> materials(diffuseTextures.size());
  for (uint32_t i = 0; i < diffuseTextures.size(); i++)
    materials[i].diffuse = diffuseTextures[i];

  Renderer::loadModel(name, meshes, materials);

  return true;
}

} // namespace Ash::Helper
