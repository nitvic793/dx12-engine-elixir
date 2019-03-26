#include "stdafx.h"
#include "ModelLoader.h"
#include <map>
#include "Utility.h"
#include <unordered_map>
#include <stack>
#include <queue>
#include "Animation.h"
#include "OGLMath.h"

ModelLoader* ModelLoader::Instance = nullptr;
Assimp::Importer ModelLoader::importer;

void TransformChannel(aiNodeAnim* animNode, AnimationChannel& channel)
{
	channel.NodeName = std::string(animNode->mNodeName.data);
	channel.PositionKeys.resize(animNode->mNumPositionKeys);
	channel.RotationKeys.resize(animNode->mNumRotationKeys);
	channel.ScalingKeys.resize(animNode->mNumScalingKeys);

	memcpy(&channel.PositionKeys[0], animNode->mPositionKeys, sizeof(aiVectorKey) * animNode->mNumPositionKeys);
	memcpy(&channel.RotationKeys[0], animNode->mRotationKeys, sizeof(aiVectorKey) * animNode->mNumRotationKeys);
	memcpy(&channel.ScalingKeys[0], animNode->mScalingKeys, sizeof(aiVectorKey) * animNode->mNumScalingKeys);
}

void LoadAnimations(const aiScene* scene, AnimationDescriptor& descriptor)
{
	//Get global inverse
	ogldev::Matrix4f m_GlobalInverseTransform = scene->mRootNode->mTransformation;
	m_GlobalInverseTransform.Inverse();
	auto GlobalInverse = XMMatrixTranspose(OGLtoXM(m_GlobalInverseTransform));
	XMStoreFloat4x4(&descriptor.GlobalInverseTransform, GlobalInverse);

	descriptor.RootNode = std::string(scene->mRootNode->mName.data);
	descriptor.Animations.resize(scene->mNumAnimations);
	std::queue<aiNode*> nodeQueue;
	nodeQueue.push(scene->mRootNode);
	XMFLOAT4X4 transform;

	//Flatten Heirarchy and map node heirarchy 
	while (!nodeQueue.empty()) 
	{
		auto node = nodeQueue.front();
		auto name = std::string(node->mName.data);
		ogldev::Matrix4f Transformation(node->mTransformation);
		XMMATRIX NodeTransformation = XMMatrixTranspose(OGLtoXM(Transformation));
		if (descriptor.NodeHeirarchy.find(name) == descriptor.NodeHeirarchy.end()) //If node not in heirarchy
		{
			std::vector<std::string> children;
			children.resize(node->mNumChildren);
			for (auto i = 0u; i < node->mNumChildren; ++i)
			{
				auto child = node->mChildren[i];
				auto childName = std::string(child->mName.data);
				children[i] = childName;
				nodeQueue.push(child);
			}

			XMStoreFloat4x4(&transform, NodeTransformation);
			descriptor.NodeTransformsMap.insert(std::pair<std::string, XMFLOAT4X4>(name, transform)); //Store node transform
			descriptor.NodeHeirarchy.insert(std::pair<std::string, std::vector<std::string>>(name, children));
		}

		nodeQueue.pop();
	}

	//Load animations
	auto& anims = descriptor.Animations;
	for (auto i = 0u; i < scene->mNumAnimations; ++i)
	{
		Animation animation;
		animation.Channels.resize(scene->mAnimations[i]->mNumChannels);
		for (auto cIndex = 0u; cIndex < animation.Channels.size(); ++cIndex)
		{
			TransformChannel(scene->mAnimations[i]->mChannels[cIndex], animation.Channels[cIndex]);
		}
		anims[i] = animation;
	}

	for (auto& anim : anims)
	{
		auto index = 0u;
		for (auto& channel : anim.Channels)
		{
			anim.NodeChannelMap.insert(std::pair<std::string, uint32_t>(channel.NodeName, index));
			index++;
		}
	}
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

	std::map<std::string, uint32_t> boneMapping;
	std::vector<BoneInfo> boneInfoList;
	std::vector<VertexBoneData> bones;

	if (mesh->HasBones())
	{
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
	if (mesh->HasBones()) 
	{
		outMesh->InitializeBoneWeights(index, BoneDescriptor{ boneMapping, boneInfoList, bones }, clist);
	}
	return outMesh;
}

void ModelLoader::ProcessMesh(UINT index, aiMesh * mesh, const aiScene * scene, std::vector<MeshEntry> meshEntries, std::vector<Vertex>& vertices, std::vector<UINT>& indices)
{
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
}

void ModelLoader::LoadBones(UINT index, const aiMesh * mesh, const aiScene * scene, std::vector<MeshEntry> meshEntries, std::map<std::string, uint32_t>& boneMapping, std::vector<BoneInfo>& boneInfoList, std::vector<VertexBoneData>& bones)
{
	if (mesh->HasBones())
	{
		auto globalTransform = aiMatrixToXMFloat4x4(&scene->mRootNode->mTransformation);
		XMFLOAT4X4 invGlobalTransform;
		XMStoreFloat4x4(&invGlobalTransform, XMMatrixInverse(nullptr, XMLoadFloat4x4(&globalTransform)));
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
			boneInfoList[boneIndex].Offset = mesh->mBones[i]->mOffsetMatrix;

			for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
			{
				uint32_t vertexID = meshEntries[index].BaseVertex + mesh->mBones[i]->mWeights[j].mVertexId;
				float weight = mesh->mBones[i]->mWeights[j].mWeight;
				bones[vertexID].AddBoneData(boneIndex, weight);
			}
		}
	}
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
	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded | aiProcess_ValidateDataStructure | aiProcess_JoinIdenticalVertices);

	if (pScene == NULL)
		return nullptr;

	UINT NumVertices = 0;
	UINT NumIndices = 0;
	std::vector<MeshEntry> meshEntries(pScene->mNumMeshes);
	for (UINT i = 0; i < meshEntries.size(); i++) 
	{
		meshEntries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		meshEntries[i].BaseVertex = NumVertices;
		meshEntries[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += meshEntries[i].NumIndices;
	}

	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	std::map<std::string, uint32_t> boneMapping;
	std::vector<BoneInfo> boneInfoList;
	std::vector<VertexBoneData> bones;
	bones.resize(NumVertices);

	Mesh* mesh = new Mesh(device, 1, pScene->HasAnimations(), pScene);
	
	for (UINT i = 0; i < pScene->mNumMeshes; ++i)
	{
		ProcessMesh(i, pScene->mMeshes[i], pScene, meshEntries, vertices, indices);
		LoadBones(i, pScene->mMeshes[i], pScene, meshEntries, boneMapping, boneInfoList, bones);
	}

	mesh->Initialize(0, vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size(), clist);
	if (pScene->HasAnimations())
	{
		mesh->mAiScene = importer.GetOrphanedScene();
		mesh->InitializeBoneWeights(0, BoneDescriptor{ boneMapping, boneInfoList, bones }, clist);
		LoadAnimations(pScene, mesh->Animations);
	}

	return mesh;
}

Mesh* ModelLoader::LoadFile(std::string filename, ID3D12GraphicsCommandList* clist)
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
