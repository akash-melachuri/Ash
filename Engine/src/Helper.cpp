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

std::vector<std::string> loadTextures(const std::string &name,
                                      const std::string &directory,
                                      aiMaterial *mat, aiTextureType type,
                                      const std::string &typeName) {
  std::vector<std::string> textures;
  textures.reserve(mat->GetTextureCount(type));
  for (uint32_t i = 0; i < mat->GetTextureCount(type); i++) {
    aiString path;
    mat->GetTexture(type, i, &path);
    Renderer::loadTexture(name + typeName + std::to_string(i),
                          directory + std::string(path.C_Str()));
    textures.emplace_back(name + typeName + std::to_string(i));
  }

  return textures;
}

void processNode(const aiNode *node, const aiScene *scene,
                 const std::string &name, const std::string &directory,
                 std::vector<std::string> &meshes,
                 std::vector<std::string> &textures) {
  for (uint32_t i = 0; i < node->mNumMeshes; i++) {
    std::string mesh_name = name + "_" + std::to_string(node->mMeshes[i]);
    if (Renderer::hasMesh(mesh_name))
      continue;
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    processMesh(mesh, mesh_name);
    meshes.push_back(mesh_name);
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      std::vector<std::string> texs = loadTextures(
          name, directory, material, aiTextureType_DIFFUSE, "diffuse");
      textures.insert(textures.end(), texs.begin(), texs.end());
    } else {
      ASH_INFO("Using backup texture");
      textures.push_back("white");
    }
  }

  for (uint32_t i = 0; i < node->mNumChildren; i++)
    processNode(node->mChildren[i], scene, name, directory, meshes, textures);
}

bool importModel(const std::string &name, const std::string &file) {
  Assimp::Importer importer;

  const aiScene *scene =
      importer.ReadFile(file, aiProcess_Triangulate | aiProcess_PreTransformVertices |
                                  aiProcess_OptimizeMeshes);

#ifdef ASH_WINDOWS
  char separator = '\\';
#else
  char separator = '/';
#endif

  std::string directory = file.substr(0, file.find_last_of(separator) + 1);

  ASH_ASSERT(scene, "Failed to import mesh {}", file);

  std::vector<std::string> meshes;
  std::vector<std::string> textures;
  Helper::processNode(scene->mRootNode, scene, name, directory, meshes,
                      textures);
  Renderer::loadModel(name, meshes, textures);

  return true;
}

} // namespace Ash::Helper
