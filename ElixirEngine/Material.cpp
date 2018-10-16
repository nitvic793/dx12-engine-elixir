#include "stdafx.h"
#include "Material.h"



Material::Material(CDescriptorHeapWrapper& heap, std::vector<std::wstring> textureList, ID3D12Device * device, ID3D12CommandQueue* commandQueue, int startIndex)
{
	this->startIndex = startIndex;
	descriptorHeap = heap;
	ResourceUploadBatch uploadBatch(device);
	uploadBatch.Begin();
	CreateWICTextureFromFile(device, uploadBatch, textureList[MATERIAL_ALBEDO].c_str(), &albedoTexture, false);
	CreateWICTextureFromFile(device, uploadBatch, textureList[MATERIAL_NORMAL].c_str(), &normalTexture, true);
	CreateWICTextureFromFile(device, uploadBatch, textureList[MATERIAL_ROUGHNESS].c_str(), &roughnessTexture, true);
	CreateWICTextureFromFile(device, uploadBatch, textureList[MATERIAL_METALNESS].c_str(), &metalnessTexture, true);
	auto uploadOperation = uploadBatch.End(commandQueue);
	uploadOperation.wait();

	CreateShaderResourceView(device, albedoTexture, heap.handleCPU(startIndex + MATERIAL_ALBEDO));
	CreateShaderResourceView(device, normalTexture, heap.handleCPU(startIndex + MATERIAL_NORMAL));
	CreateShaderResourceView(device, roughnessTexture, heap.handleCPU(startIndex + MATERIAL_ROUGHNESS));
	CreateShaderResourceView(device, metalnessTexture, heap.handleCPU(startIndex + MATERIAL_METALNESS));
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetGPUDescriptorHandle()
{
	return descriptorHeap.handleGPU(startIndex);
}


Material::~Material()
{
	albedoTexture->Release();
	normalTexture->Release();
	roughnessTexture->Release();
	metalnessTexture->Release();
}
