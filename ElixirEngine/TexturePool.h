#pragma once

#include "stdafx.h"
#include "Texture.h"
#include "DirectXHelper.h"

class TexturePool
{
	std::vector<ID3D12Resource*> textures;
	std::vector<Texture*> textureSRVs;
	std::vector<Texture*> textureUAVs;
	int maxTextureCount;

	ID3D12Device* device;
	CDescriptorHeapWrapper* descriptorHeap;
	DeferredRenderer* renderContext;

	void AddTexture(DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT, int width = 1280, int height = 720);
public:
	TexturePool(ID3D12Device* device, DeferredRenderer* renderContext, int totalTextureCount = 6);
	Texture* GetSRV(int index);
	Texture* GetUAV(int index);
	~TexturePool();
};

