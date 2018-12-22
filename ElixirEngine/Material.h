#pragma once
#include "stdafx.h"
#include "DirectXHelper.h"
#include "StringHash.h"

class DeferredRenderer;

enum MaterialTextureType {
	MATERIAL_ALBEDO = 0,
	MATERIAL_NORMAL,
	MATERIAL_ROUGHNESS,
	MATERIAL_METALNESS,
	MATERIAL_COUNT
};

struct MaterialLoadData
{
	HashID MaterialID;
	std::wstring AlbedoFile;
	std::wstring NormalFile;
	std::wstring RoughnessFile;
	std::wstring MetalnessFile;
};

class Material
{
	ID3D12Resource* albedoTexture;
	ID3D12Resource* normalTexture;
	ID3D12Resource* roughnessTexture;
	ID3D12Resource* metalnessTexture;
	CDescriptorHeapWrapper descriptorHeap;
	uint32_t startIndex;
public:
	Material(CDescriptorHeapWrapper& heap, std::vector<std::wstring> textureList, ID3D12Device* device, ID3D12CommandQueue* commandQueue, int startIndex);
	Material(DeferredRenderer* renderContext, std::vector<std::wstring> textureList, ID3D12Device* device, ID3D12CommandQueue* commandQueue);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle();
	~Material();
};

