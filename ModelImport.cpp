#include "ModelImport.hpp"

namespace TLB
{
    void ModelImport::ImportPath(const std::string& path, bool importBoneData)
    {
        meshes.clear();
        this->path = path;
        withBoneData = importBoneData;
        boneCounter = 0;
        fullImport = false;

        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "[ModelImport] ERROR/ASSIMP:[" << import.GetErrorString() << "]\n"; return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        ProcessNode(scene->mRootNode, scene);
        fullImport = true;
    }

    void ModelImport::ProcessNode(aiNode* node, const aiScene* scene)
    {
        //process the node's meshes if any
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(ProcessMesh(mesh, scene));
        }

        //and process the node's children nodes
        if (node->mNumChildren > 0)
        {
            for (uint32_t i = 0; i < node->mNumChildren; i++)
            {
                ProcessNode(node->mChildren[i], scene);
            }
        }
    }

    MeshImport ModelImport::ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        MeshImport Imesh;
        Imesh.verts.reserve(mesh->mNumVertices);
        Imesh.inds.reserve(mesh->mNumFaces * 3);
        Imesh.uvs.reserve(mesh->mNumVertices);
        Imesh.norms.reserve(mesh->mNumVertices);
        Imesh.boneWeights.reserve(mesh->mNumVertices * MAX_BONE_WEIGHTS  * withBoneData);

        //Vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Imesh.verts.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->mNormals != nullptr)
            {
                Imesh.norms.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            else Imesh.norms.emplace_back(0, 0, 0);

            if (mesh->mTextureCoords[0])
            {
                Imesh.uvs.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }
            else Imesh.uvs.emplace_back(0, 0);
        }

        //Indices
        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++)
            {
                Imesh.inds.push_back(face.mIndices[j]);
            }
        }

        //Texture
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
                Imesh.txtr = directory + '/' + str.C_Str();
            }
        }

        //Bones
        if(withBoneData) ExtractBoneWeightForVertices(Imesh, mesh, scene);

        return Imesh;
    }

    void ModelImport::ExtractBoneWeightForVertices(MeshImport& Imesh, aiMesh* mesh, const aiScene* scene)
    {
        for (int i = 0; i < mesh->mNumVertices * MAX_BONE_WEIGHTS; i++)
        {
            Imesh.boneWeights.emplace_back(-1, 0.f); //Setting default values
        }

        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

            boneID = boneCounter;
            boneCounter++;

            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= Imesh.verts.size());

                // Set ID and weight to the first empty slot if available
                for (int i = vertexId * MAX_BONE_WEIGHTS; i < (vertexId + 1) * MAX_BONE_WEIGHTS; ++i)
                {
                    if (Imesh.boneWeights[i].first < 0)
                    {
                        Imesh.boneWeights[i].first = boneID;
                        Imesh.boneWeights[i].second = weight;
                        break;
                    }
                }
            }
        }

    }
}