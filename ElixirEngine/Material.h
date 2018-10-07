#pragma once
#include "stdafx.h"
#include "DirectXHelper.h"

enum MaterialTextureType {
	MATERIAL_ALBEDO = 0,
	MATERIAL_NORMAL,
	MATERIAL_ROUGHNESS,
	MATERIAL_METALNESS,
	MATERIAL_COUNT
};

class Material
{
	ID3D12Resource* albedoTexture;
	ID3D12Resource* normalTexture;
	ID3D12Resource* roughnessTexture;
	ID3D12Resource* metalnessTexture;
	CDescriptorHeapWrapper descriptorHeap;
	int startIndex;
public:
	Material(CDescriptorHeapWrapper& heap, std::vector<std::wstring> textureList, ID3D12Device* device, ID3D12CommandQueue* commandQueue, int startIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle();
	~Material();
};

