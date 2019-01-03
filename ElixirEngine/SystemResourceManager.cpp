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

SystemResourceManager::~SystemResourceManager()
{
	for (auto& p : psoMap)
	{
		p.second->Release();
	}
}
