#include "stdafx.h"
#include "ModelLoader.h"
#include <map>

const static int CNumBonesPerVertex = 4;
ModelLoader* ModelLoader::Instance = nullptr;

struct BoneInfo
{
	XMFLOAT4X4 OffsetMatrix;
};

struct VertexBoneData
{
	uint32_t IDs[CNumBonesPerVertex];
	float Weights[CNumBonesPerVertex];

	void AddBoneData(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < CNumBonesPerVertex; i++) {
			if (Weights[i] == 0.0) {
				IDs[i] = boneID;
				Weights[i] = weight;
				return;
			}
		}
	}
};

XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe)
{
	XMFLOAT4X4 output;
	output._11 = aiMe->a1;
	output._12 = aiMe->a2;
	output._13 = aiMe->a3;
	output._14 = aiMe->a4;

	output._21 = aiMe->b1;
	output._22 = aiMe->b2;
	output._23 = aiMe->b3;
	output._24 = aiMe->b4;

	output._31 = aiMe->c1;
	output._32 = aiMe->c2;
	output._33 = aiMe->c3;
	output._34 = aiMe->c4;

	output._41 = aiMe->d1;
	output._42 = aiMe->d2;
	output._43 = aiMe->d3;
	output._44 = aiMe->d4;

	return output;
}

Mesh* ModelLoader::ProcessMesh(UINT index, aiMesh* mesh, const aiScene * scene, Mesh* &outMesh, ID3D12GraphicsCommandList* clist)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	// Walk through each of the mesh's vertices
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		vertex.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		if (mesh->mTextureCoords[0])
		{
			vertex.uv.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.uv.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		auto c = material->GetTextureCount(aiTextureType_DIFFUSE);
	}

	if (mesh->HasBones())
	{
		std::map<std::string, uint32_t> boneMapping;
		std::vector<BoneInfo> boneInfoList;
		std::vector<VertexBoneData> bones;
		auto globalTransform = aiMatrixToXMFloat4x4(&scene->mRootNode->mTransformation);
		XMFLOAT4X4 invGlobalTransform;
		XMStoreFloat4x4(&invGlobalTransform, XMMatrixInverse(nullptr, XMLoadFloat4x4(&globalTransform)));
		bones.resize(vertices.size());
		uint32_t numBones = 0;
		for (uint32_t i = 0; i < mesh->mNumBones; i++)
		{
			uint32_t boneIndex = 0;
			std::string boneName(mesh->mBones[i]->mName.data);
			if (boneMapping.find(boneName) == boneMapping.end()) //if bone not found
			{
				boneIndex = numBones;
				numBones++;
				BoneInfo bi = {};
				boneInfoList.push_back(bi);
			}
			else
			{
				boneIndex = boneMapping[boneName];
			}

			boneMapping[boneName] = boneIndex;
			boneInfoList[boneIndex].OffsetMatrix = aiMatrixToXMFloat4x4(&mesh->mBones[i]->mOffsetMatrix);

			for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
			{
				uint32_t vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
				float weight = mesh->mBones[i]->mWeights[j].mWeight;
				bones[vertexID].AddBoneData(boneIndex, weight);
			}
		}
	}

	outMesh->Initialize(index, vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size(), clist);
	return outMesh;
}

void ModelLoader::ProcessNode(aiNode * node, const aiScene * scene, ID3D12GraphicsCommandList* clist, Mesh* &outMesh)
{
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(i, mesh, scene, outMesh, clist);
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, clist, outMesh);
	}
}

Mesh* ModelLoader::Load(std::string filename, ID3D12GraphicsCommandList* clist)
{
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	if (pScene == NULL)
		return nullptr;
	Mesh* mesh = new Mesh(device, pScene->mNumMeshes);
	for (UINT i = 0; i < pScene->mNumMeshes; ++i)
	{
		ProcessMesh(i, pScene->mMeshes[i], pScene, mesh, clist);
	}
	//ProcessNode(pScene->mRootNode, pScene, clist, mesh);

	return mesh;
}

Mesh*ModelLoader::LoadFile(std::string filename, ID3D12GraphicsCommandList* clist)
{
	return Instance->Load(filename, clist);
}

ModelLoader * ModelLoader::GetInstance()
{
	return Instance;
}

ModelLoader * ModelLoader::CreateInstance(ID3D12Device * device)
{
	if (!Instance)
	{
		Instance = new ModelLoader(device);
	}
	return Instance;
}

void ModelLoader::DestroyInstance()
{
	delete Instance;
}

ModelLoader::ModelLoader(ID3D12Device * device) :
	device(device)
{
}
