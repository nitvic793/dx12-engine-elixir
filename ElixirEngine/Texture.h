#pragma once
#include "stdafx.h"
#include "DirectXHelper.h"

class DeferredRenderer;

enum TextureViewType
{
	TextureTypeSRV = 0,
	TextureTypeUAV
};

enum TextureFileType
{
	TexFileTypeWIC = 0,
	TexFileTypeDDS
};

class Texture
{
	ID3D12Device*			device;
	ID3D12Resource*			resource;
	CDescriptorHeapWrapper* descriptorHeap;
	TextureViewType			textureViewType;
	uint32_t				heapIndex;
	DeferredRenderer*		renderContext;

	bool isCubeMap;
	bool releaseNeeded;
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
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle();
	CDescriptorHeapWrapper*		GetTextureDescriptorHeap();
	ID3D12Resource*				GetTextureResource();

	Texture(DeferredRenderer* renderContext, ID3D12Device* device);
	Texture(DeferredRenderer* renderContext, ID3D12Device* device, ID3D12Resource* resource, int heapIndex, TextureViewType viewType);
	Texture(DeferredRenderer* renderContext, ID3D12Device* device, ID3D12Resource* resource, int heapIndex, TextureViewType viewType, CDescriptorHeapWrapper* descHeap);
	~Texture();
};

