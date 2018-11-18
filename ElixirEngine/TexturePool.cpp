#include "stdafx.h"
#include "TexturePool.h"
#include "Core/DeferredRenderer.h"


int64_t hash(int x_, int y_, int z_) {
	uint64_t result = uint16_t(x_);
	result = (result << 16) + uint16_t(y_);
	result = (result << 16) + uint16_t(z_);
	return result;
}

void TexturePool::AddTexture(DXGI_FORMAT format, int width, int height)
{
	ID3D12Resource* texture;
	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resourceDesc.Format = format;

	float mClearColor[4] = { 0.0,0.0f,0.0f,1.0f };
	D3D12_CLEAR_VALUE clearVal;
	clearVal.Color[0] = mClearColor[0];
	clearVal.Color[1] = mClearColor[1];
	clearVal.Color[2] = mClearColor[2];
	clearVal.Color[3] = mClearColor[3];
	clearVal.Format = format;

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, &clearVal, IID_PPV_ARGS(&texture));
	textures.push_back(texture);
	auto heapIndex = renderContext->SetSRV(texture, format);
	textureSRVs.push_back(new Texture(renderContext, device, texture, heapIndex, TextureTypeSRV));

	heapIndex = renderContext->SetUAV(texture, false, format);
	textureUAVs.push_back(new Texture(renderContext, device, texture, heapIndex, TextureTypeUAV));

	D3D12_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Format = format;
	device->CreateRenderTargetView(texture, &desc, rtvHeap.handleCPU((UINT)textures.size() - 1));
}

TexturePool::TexturePool(ID3D12Device* device, DeferredRenderer* renderContext, int totalTextureCount)
{
	this->device = device;
	this->renderContext = renderContext;
	descriptorHeap = &renderContext->GetSRVHeap();
	rtvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, totalTextureCount);

	for (int i = 0; i < totalTextureCount; ++i)
	{
		AddTexture();
	}

}

Texture * TexturePool::GetSRV(int index)
{
	return textureSRVs[index];
}

Texture * TexturePool::GetUAV(int index)
{
	return textureUAVs[index];
}

D3D12_CPU_DESCRIPTOR_HANDLE TexturePool::GetRTVHandle(int index)
{
	return rtvHeap.handleCPU(index);
}

Texture* TexturePool::Request(DXGI_FORMAT format, int width, int height, TextureViewType type, int* index, bool getCached)
{
	int texIndex;
	auto hashed = hash(format, width, height);
	bool containsKey = textureRequestMap.find(hashed) != textureRequestMap.end();
	if (!containsKey || !getCached)
	{
		texIndex = maxTextureCount;
		AddTexture(format, width, height);
		if (!containsKey)
			textureRequestMap.insert(std::pair<uint64_t, int>(hashed, maxTextureCount));
		else
			textureRequestMap[hashed] = maxTextureCount;
		maxTextureCount++;
	}
	else
	{
		texIndex = textureRequestMap[hashed];
	}

	if (index != nullptr)
		*index = texIndex;

	if (type == TextureTypeSRV)
		return GetSRV(texIndex);
	else if (type == TextureTypeUAV)
		return GetUAV(texIndex);

	return nullptr;
}


TexturePool::~TexturePool()
{
	rtvHeap.pDescriptorHeap->Release();
	for (auto t : textureSRVs)
		delete t;

	for (auto t : textureUAVs)
		delete t;

	for (auto &t : textures)
		t->Release();
}
