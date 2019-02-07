#include "stdafx.h"
#include "ModelLoader.h"

ModelLoader* ModelLoader::Instance = nullptr;

Mesh* ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene * scene, ID3D12GraphicsCommandList* clist)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	//std::vector<Texture> textures;
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

	auto elixMesh = new Mesh(device);
	elixMesh->Initialize(vertices.data(), (UINT)vertices.size(), indices.data(), (UINT)indices.size(), clist);
	return elixMesh;
}

void ModelLoader::ProcessNode(aiNode * node, const aiScene * scene, ID3D12GraphicsCommandList* clist, std::vector<Mesh*> &meshes)
{
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene, clist));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, clist, meshes);
	}
}

std::vector<Mesh*> ModelLoader::Load(std::string filename, ID3D12GraphicsCommandList* clist)
{
	Assimp::Importer importer;
	std::vector<Mesh*> meshes;
	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	if (pScene == NULL)
		return meshes;

	ProcessNode(pScene->mRootNode, pScene, clist, meshes);

	return meshes;
}



std::vector<Mesh*> ModelLoader::LoadFile(std::string filename, ID3D12GraphicsCommandList* clist)
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

ModelLoader::ModelLoader(ID3D12Device * device) :
	device(device)
{
}
