#pragma once
#include "stdafx.h"
#include "Texture.h"
#include "Core/Mesh.h"
#include "Material.h"
#include <unordered_map>
#include "../Engine.Serialization/StringHash.h"
#include <typeinfo>
#include "../Engine.Serialization/SceneSerDe.h"
#include "../Engine.Serialization/ResourcePackSerDe.h"

#include "Core/Entity.h"

// Engine Specific
typedef std::unordered_map<unsigned int, Mesh*> MeshMap;
typedef std::unordered_map<unsigned int, Material*> MaterialMap;
typedef std::unordered_map<unsigned int, Texture*> TextureMap;

struct TextureLoadData
{
	HashID TextureID;
	std::wstring FilePath;
	TextureFileType FileType;
	bool IsCubeMap;
};

class ResourceManager
{
private:
	static ResourceManager* Instance;

protected:
	ID3D12Device*	device;
	MeshMap			meshes;
	MaterialMap		materials;
	TextureMap		textures;
	ResourceManager(ID3D12Device* device);
public:
	static ResourceManager* CreateInstance(ID3D12Device* device);
	static ResourceManager* GetInstance();

	void LoadTexture(
		ID3D12CommandQueue* commandQueue,
		DeferredRenderer* renderer,
		HashID textureID, 
		std::wstring filepath,
		TextureFileType texFileType,
		bool isCubeMap = false,
		TextureViewType viewType = TextureTypeSRV
	);
	void LoadTextures(ID3D12CommandQueue* commandQueue, DeferredRenderer* renderer, std::vector<TextureLoadData> textureLoadData);

	void LoadMaterial(ID3D12CommandQueue* commandQueue, DeferredRenderer* renderer, MaterialLoadData loadData);
	void LoadMaterials(ID3D12CommandQueue* commandQueue, DeferredRenderer* renderer, std::vector<MaterialLoadData> materials);

	void LoadMeshes(ID3D12GraphicsCommandList* commandList, std::vector<std::string> meshList);
	void LoadMesh(ID3D12GraphicsCommandList* commandList, std::string filePath);
	void LoadMesh(ID3D12GraphicsCommandList* commandList, HashID hashId, std::string filePath);

	Mesh*		GetMesh(HashID hashId);
	Material*	GetMaterial(HashID materialID);
	Texture*	GetTexture(HashID textureID);
	Scene		LoadScene(std::string filename, std::vector<Entity*> &outEntities);
	void		LoadResources(std::string filename, ID3D12CommandQueue* cqueue, ID3D12GraphicsCommandList* clist, DeferredRenderer* renderer);
	~ResourceManager();
};

/*TODO*/

class IMemoryPool
{
};

template<typename T>
class MemoryPool : public IMemoryPool
{
public:
	T* Allocate()
	{
		return nullptr;
	}

	void Destroy(T* block)
	{

	}
};

class PoolAllocator
{
	std::unordered_map<size_t, IMemoryPool*> memoryPools;
public:
	template<typename T>
	void RegisterType() 
	{
		memoryPools.insert(std::pair<size_t, IMemoryPool*>(typeid(T).hash_code(), new MemoryPool<T>()));
	}

	template<typename T>
	T* GetNew()
	{
		auto pool = (MemoryPool<T>*)memoryPools[typeid(T).hash_code()];
		return pool->Allocate();
	}

	template<typename T>
	void Destroy(T* block)
	{
		auto pool = (MemoryPool<T>*)memoryPools[typeid(T).hash_code()];
		pool->Destroy(block);
	}

	~PoolAllocator()
	{
		for (auto& pool : memoryPools)
		{
			delete pool.second;
		}
	}
};

