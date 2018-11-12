#include "stdafx.h"
#include "ComputeProcess.h"


void ComputeProcess::CreatePSO()
{
	auto device = computeCore->GetDevice();
	auto rootSignature = computeCore->GetRootSignature();
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = rootSignature;
	computePsoDesc.CS = ShaderManager::LoadShader(shaderFileName.c_str());

	device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&pipelineStateObject));
	pipelineStateObject->SetName(shaderFileName.c_str());
}

ComputeProcess::ComputeProcess(ComputeCore* core, std::wstring filename) :
	shaderFileName(filename),
	computeCore(core)
{
	CreatePSO();
}


ComputeProcess::~ComputeProcess()
{
	if (pipelineStateObject)pipelineStateObject->Release();
}

void ComputeProcess::SetShader(ID3D12GraphicsCommandList * commandList)
{
	auto rootSignature = computeCore->GetRootSignature();
	commandList->SetComputeRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineStateObject);
}

void ComputeProcess::SetTextureUAV(ID3D12GraphicsCommandList* commandList, Texture* textureUAV)
{
	ID3D12DescriptorHeap* ppHeaps[] = { textureUAV->GetTextureDescriptorHeap()->pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, ppHeaps);
	commandList->SetComputeRootDescriptorTable(1, textureUAV->GetGPUDescriptorHandle());
}

void ComputeProcess::SetTextureSRV(ID3D12GraphicsCommandList* commandList, Texture* textureSRV)
{
	ID3D12DescriptorHeap* ppHeaps[] = { textureSRV->GetTextureDescriptorHeap()->pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, ppHeaps);
	commandList->SetComputeRootDescriptorTable(0, textureSRV->GetGPUDescriptorHandle());
}

void ComputeProcess::SetTextureSRVOffset(ID3D12GraphicsCommandList * commandList, Texture * textureSRV)
{
	ID3D12DescriptorHeap* ppHeaps[] = { textureSRV->GetTextureDescriptorHeap()->pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, ppHeaps);
	commandList->SetComputeRootDescriptorTable(3, textureSRV->GetGPUDescriptorHandle());
}

void ComputeProcess::SetConstants(ID3D12GraphicsCommandList* commandList, void * data, UINT count, UINT offset)
{
	commandList->SetComputeRoot32BitConstants(2, count, data, offset); // constant buffer value settings
}

void ComputeProcess::Dispatch(ID3D12GraphicsCommandList* commandList, int x, int y, int z)
{
	commandList->Dispatch(x, y, z);
}
