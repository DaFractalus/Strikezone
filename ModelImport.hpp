#pragma once

namespace TLB
{
	constexpr int MAX_BONE_WEIGHTS = 4;
	struct MeshImport
	{
		vector<uint> inds;
		vector<glm::vec3> verts;
		vector<glm::vec3> norms;
		vector<glm::vec2> uvs;
		vector<std::pair<int, float>> boneWeights; // <boneIndex, weight>, each vertex gets MAX_BONE_WEIGHTS sequential values, default is set to <0, -1>
		std::string txtr;

	};

	struct ModelImport
	{
		vector<MeshImport> meshes;
		std::string path;
		std::string directory;
		bool fullImport, withBoneData;
		int boneCounter = 0; // ???

		ModelImport() : fullImport(false) {}
		void ImportPath(const std::string& path, bool importBoneData);
		void ProcessNode(aiNode* node, const aiScene* scene);
		MeshImport ProcessMesh(aiMesh* mesh, const aiScene* scene);
		void ExtractBoneWeightForVertices(MeshImport& Imesh, aiMesh* mesh, const aiScene* scene);
	};
}