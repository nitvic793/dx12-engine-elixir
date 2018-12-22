#pragma once
#include "stdafx.h"
#include "Texture.h"
#include "Core/Mesh.h"
#include "Material.h"
#include <unordered_map>
#include "StringHash.h"

// Engine Specific
typedef std::unordered_map<unsigned int, Mesh*> MeshMap;
typedef std::unordered_map<unsigned int, Material*> MaterialMap;
typedef std::unordered_map<unsigned int, Texture*> TextureMap;

// DirectX 12 specific
typedef std::unordered_map<unsigned int, ID3D12PipelineState*> PSOMap;
typedef std::unordered_map<unsigned int, ID3D12RootSignature*> RootSigMap;
typedef std::unordered_map<unsigned int, ID3D12Resource*> ResourceMap;
typedef std::unordered_map<unsigned int, CDescriptorHeapWrapper> DescriptorHeapMap;
typedef std::unordered_map<unsigned int, ConstantBufferWrapper> ConstantBufferMap;

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
	ID3D12Device* device;
	MeshMap meshes;
	MaterialMap materials;
	TextureMap textures;
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

	Mesh* GetMesh(HashID hashId);
	Material* GetMaterial(HashID materialID);
	Texture* GetTexture(HashID textureID);
	~ResourceManager();
};

