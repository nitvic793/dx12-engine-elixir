#include "stdafx.h"
#include "Texture.h"


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
	heapIndex = renderContext->SetSRV(resource, isCubeMap);
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGPUDescriptorHandle()
{
	return descriptorHeap->handleGPU(heapIndex);
}

Texture::Texture(DeferredRenderer* renderContext, ID3D12Device* device)
{
	descriptorHeap = &renderContext->GetSRVHeap();
	this->device = device;
}


Texture::~Texture()
{
}
