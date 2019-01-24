#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImagePlus.h> 
#include <string.h>
#include "assets.h"

#include <iostream>
int32_t Assets::packNormals(glm::vec3 normal) const
{
    normal = glm::normalize(normal);
    glm::ivec3 scaledNormal = glm::ivec3(   static_cast<int>(normal.x*511),
                                            static_cast<int>(normal.y*511),
                                            static_cast<int>(normal.z*511));
    return (scaledNormal.x << 20) | (scaledNormal.y << 10) | scaledNormal.z;
}

std::shared_ptr<Assets::Model> Assets::loadModel(const char *path) const
{
    auto model = std::make_shared<Model>();

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_OptimizeMeshes ); 
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        throw std::runtime_error(importer.GetErrorString());
    std::cerr <<importer.GetErrorString();

    //assuming only one mesh in file for now, do we need more?
    aiMesh *mesh = scene->mMeshes[0];
    for(unsigned int v=0; v<mesh->mNumVertices; v++)
    { 
        model->vertices.push_back(Vertex{   glm::vec3(  mesh->mVertices[v].x,
                                                        mesh->mVertices[v].y,
                                                        mesh->mVertices[v].z),
                                            packNormals(glm::vec3(  mesh->mNormals[v].x,
                                                                    mesh->mNormals[v].y,
                                                                    mesh->mNormals[v].z)),
                                            glm::packSnorm2x16(glm::vec2(   mesh->mTextureCoords[0][v].x,
                                                                            mesh->mTextureCoords[0][v].y)) 
                                            }); 
    } 

    for(unsigned int f=0; f<mesh->mNumFaces; f++)
    {
        aiFace face = mesh->mFaces[f];
        for(unsigned int i=0; i<face.mNumIndices; i++)
            model->indices.push_back(face.mIndices[i]);               
    }
 
    return model;
}

std::shared_ptr<Assets::Texture> Assets::loadTexture(const char *path) const
{
    auto texture = std::make_shared<Texture>();

    fipImage image;
    if(!image.load(path))
       throw std::runtime_error("Cannot load texture:" + std::string(path)); 
    if (!image.convertTo32Bits())
       throw std::runtime_error("Cannot convert (to 32 bits) texture:" + std::string(path)); 

    texture->width = image.getWidth();
    texture->height = image.getHeight();
    unsigned int size = texture->width*texture->height;
    texture->pixels.reserve(size*Texture::BYTES_PER_PIXEL);

    memcpy(texture->pixels.data(), image.accessPixels(), size);

    return texture;
}
