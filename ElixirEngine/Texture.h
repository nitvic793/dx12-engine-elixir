#pragma once
#include "stdafx.h"
#include "DirectXHelper.h"
#include "Core/DeferredRenderer.h"

enum TextureViewType
{
	TextureTypeSRV,
	TextureTypeUAV
};

enum TextureFileType
{
	TexFileTypeWIC,
	TexFileTypeDDS
};

class Texture
{
	ID3D12Resource* resource;
	CDescriptorHeapWrapper* descriptorHeap;
	TextureViewType textureViewType;
	uint32_t heapIndex;
	DeferredRenderer* renderContext;
	bool isCubeMap;
	ID3D12Device* device;
public:
	void CreateTexture(std::wstring textureFileName, 
		TextureFileType textureFileType, 
		ID3D12CommandQueue* commandQueue, 
		bool isCubeMap = false, 
		TextureViewType textureViewType = TextureTypeSRV);

	void CreateTexture(std::wstring textureFileName,
		TextureFileType textureFileType,
		ID3D12CommandQueue* commandQueue,
		ResourceUploadBatch& uploadBatch,
		bool isCubeMap = false,
		TextureViewType textureViewType = TextureTypeSRV);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle();

	Texture(DeferredRenderer* renderContext, ID3D12Device* device);
	~Texture();
};

