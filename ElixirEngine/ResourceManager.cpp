#include "stdafx.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::Instance = nullptr;

ResourceManager::ResourceManager(ID3D12Device* device)
{
	this->device = device;
	Instance = this;
}

ResourceManager * ResourceManager::CreateInstance(ID3D12Device * device)
{
	ResourceManager* rm = nullptr;
	if (Instance == nullptr)
	{
		rm = new ResourceManager(device);
	}

	return rm;
}

ResourceManager * ResourceManager::GetInstance()
{
	return Instance;
}

void ResourceManager::LoadMeshes(ID3D12GraphicsCommandList* commandList, std::vector<std::string> meshList)
{
	for (auto& m : meshList)
	{
		LoadMesh(commandList, m);
	}
}

void ResourceManager::LoadMesh(ID3D12GraphicsCommandList * commandList, std::string filePath)
{
	auto filename = GetFileNameWithoutExtension(filePath);
	auto sid = StringID(filename.c_str()).GetHash();
	LoadMesh(commandList, sid, filePath);
}

void ResourceManager::LoadMesh(ID3D12GraphicsCommandList * commandList, HashID hashId, std::string filePath)
{
	auto mesh = new Mesh(filePath, device, commandList);
	meshes.insert(std::pair<HashID, Mesh*>(hashId, mesh));
}

Mesh * ResourceManager::GetMesh(HashID hashId)
{
	return meshes[hashId];
}


ResourceManager::~ResourceManager()
{
	for (auto& m : meshes)
	{
		delete m.second;
	}
}
