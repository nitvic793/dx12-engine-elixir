#include "stdafx.h"
#include "SystemResourceManager.h"

SystemResourceManager* SystemResourceManager::Instance = nullptr;


SystemResourceManager::SystemResourceManager(ID3D12Device * device)
{
	this->device = device;
	Instance = this;
}

SystemResourceManager * SystemResourceManager::CreateInstance(ID3D12Device * device)
{
	if (Instance != nullptr)
		return Instance;

	auto sRm = new SystemResourceManager(device);
	return sRm;
}

SystemResourceManager* SystemResourceManager::GetInstance()
{
	return Instance;
}

ID3D12PipelineState* SystemResourceManager::CreatePSO(HashID psoID, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc)
{
	ID3D12PipelineState* pso;
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
	psoMap.insert(std::pair<HashID, ID3D12PipelineState*>(psoID, pso));
	return pso;
}

ID3D12PipelineState* SystemResourceManager::GetPSO(HashID psoID)
{
	return psoMap[psoID];
}

ID3D12Resource * SystemResourceManager::CreateResource(
	HashID resID,
	D3D12_RESOURCE_DESC resDesc,
	ResourceType resourceType,
	D3D12_CLEAR_VALUE * clearValue,
	D3D12_HEAP_TYPE heapType,
	D3D12_RESOURCE_STATES initialState,
	D3D12_HEAP_FLAGS heapFlags)
{
	ID3D12Resource* resource;
	CD3DX12_HEAP_PROPERTIES heapProperty(heapType);
	device->CreateCommittedResource(&heapProperty, heapFlags, &resDesc, initialState, clearValue, IID_PPV_ARGS(&resource));
	resources.insert(std::pair<HashID, ID3D12Resource*>(resID, resource));
	return resource;
}

ID3D12Resource * SystemResourceManager::GetResource(HashID resID)
{
	return resources[resID];
}

SystemResourceManager::~SystemResourceManager()
{
	for (auto& p : psoMap)
	{
		p.second->Release();
	}

	for (auto& p : resources)
	{
		p.second->Release();
	}
}
