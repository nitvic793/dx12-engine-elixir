#pragma once

#include "stdafx.h"
#include "Texture.h"
#include "DirectXHelper.h"
#include <unordered_map>

struct TextureResourceBunch
{
	Texture* SRV;
	Texture* UAV;
	D3D12_CPU_DESCRIPTOR_HANDLE RTV;
};

class TexturePool
{
	std::vector<ID3D12Resource*> textures;
	std::vector<Texture*> textureSRVs;
	std::vector<Texture*> textureUAVs;
	int maxTextureCount;

	int srvIndex;
	int uavIndex;
	ID3D12Device* device;
	CDescriptorHeapWrapper* descriptorHeap;
	CDescriptorHeapWrapper rtvHeap;
	DeferredRenderer* renderContext;
	std::unordered_map<uint64_t, int> textureRequestMap;

	void AddTexture(DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT, int width = 1280, int height = 720);
public:
	TexturePool(ID3D12Device* device, DeferredRenderer* renderContext, int totalTextureCount = 6);
	Texture* GetNextSRV();
	Texture* GetNextUAV();
	TextureResourceBunch GetNext();
	Texture* GetSRV(int index);
	Texture* GetUAV(int index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(int index);
	Texture* Request(DXGI_FORMAT format, int width, int height, TextureViewType type, int *index = nullptr, bool getCached = true);

	void ResetIndex();
	~TexturePool();
};

