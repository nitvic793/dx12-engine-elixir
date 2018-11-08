#include "stdafx.h"
#include "Texture.h"
#include "Core/DeferredRenderer.h"

void Texture::CreateTexture(std::wstring textureFileName,
	TextureFileType textureFileType,
	ID3D12CommandQueue* commandQueue,
	bool isCubeMap,
	TextureViewType textureViewType)
{
	ResourceUploadBatch uploadBatch(device);
	uploadBatch.Begin();
	CreateTexture(textureFileName, textureFileType, commandQueue, uploadBatch, isCubeMap, textureViewType);
	auto uploadOperation = uploadBatch.End(commandQueue);
	uploadOperation.wait();
}

void Texture::CreateTexture(std::wstring textureFileName,
	TextureFileType textureFileType,
	ID3D12CommandQueue * commandQueue,
	ResourceUploadBatch & uploadBatch,
	bool isCubeMap,
	TextureViewType textureViewType)
{
	this->textureViewType = textureViewType;
	switch (textureFileType)
	{
	case TexFileTypeWIC:
		CreateWICTextureFromFile(device, uploadBatch, textureFileName.c_str(), &resource);
		break;
	case TexFileTypeDDS:
		CreateDDSTextureFromFile(device, uploadBatch, textureFileName.c_str(), &resource);
		break;
	}

	if (textureViewType == TextureViewType::TextureTypeSRV)
	{
		heapIndex = renderContext->SetSRV(resource, isCubeMap);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGPUDescriptorHandle()
{
	return descriptorHeap->handleGPU(heapIndex);
}

CDescriptorHeapWrapper * Texture::GetTextureDescriptorHeap()
{
	return descriptorHeap;
}

Texture::Texture(DeferredRenderer* renderContext, ID3D12Device* device)
{
	this->renderContext = renderContext;
	descriptorHeap = &renderContext->GetSRVHeap();
	this->device = device;
	releaseNeeded = true;
}

Texture::Texture(DeferredRenderer * renderContext, ID3D12Device * device, ID3D12Resource * resource, int heapIndex, TextureViewType viewType)
{
	releaseNeeded = false;
	this->renderContext = renderContext;
	descriptorHeap = &renderContext->GetSRVHeap();
	this->device = device;
	this->resource = resource;
	textureViewType = viewType;
	this->heapIndex = heapIndex;
}


Texture::~Texture()
{
	if (releaseNeeded && resource) resource->Release();
}
