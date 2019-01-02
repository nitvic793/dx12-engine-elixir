#include "stdafx.h"
#include "ComputeProcess.h"
#include "Core/DeferredRenderer.h"

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
	auto frame = computeCore->GetRenderer()->GetFrameManager();
	auto frameHeapParams = computeCore->GetRenderer()->GetFrameHeapParameters();
	ID3D12DescriptorHeap* ppHeaps[] = { frame->GetDescriptorHeap() };
	commandList->SetDescriptorHeaps(1, ppHeaps);
}

void ComputeProcess::SetTextureUAV(ID3D12GraphicsCommandList* commandList, Texture* textureUAV)
{
	auto frame = computeCore->GetRenderer()->GetFrameManager();
	auto frameHeapParams = computeCore->GetRenderer()->GetFrameHeapParameters();
	commandList->SetComputeRootDescriptorTable(ComputeSigUAV0, frame->GetGPUHandle(frameHeapParams.Textures, textureUAV->GetHeapIndex()));
}

void ComputeProcess::SetTextureSRV(ID3D12GraphicsCommandList* commandList, Texture* textureSRV)
{
	auto frame = computeCore->GetRenderer()->GetFrameManager();
	auto frameHeapParams = computeCore->GetRenderer()->GetFrameHeapParameters();
	commandList->SetComputeRootDescriptorTable(ComputeSigSRV0, frame->GetGPUHandle(frameHeapParams.Textures, textureSRV->GetHeapIndex()));
}

void ComputeProcess::SetTextureSRVOffset(ID3D12GraphicsCommandList * commandList, Texture * textureSRV)
{
	auto frame = computeCore->GetRenderer()->GetFrameManager();
	auto frameHeapParams = computeCore->GetRenderer()->GetFrameHeapParameters();
	commandList->SetComputeRootDescriptorTable(ComputeSigSRV1, frame->GetGPUHandle(frameHeapParams.Textures, textureSRV->GetHeapIndex()));
}

void ComputeProcess::SetConstants(ID3D12GraphicsCommandList* commandList, void * data, UINT count, UINT offset)
{
	commandList->SetComputeRoot32BitConstants(ComputeSigConsts0, count, data, offset); // constant buffer value settings
}

void ComputeProcess::Dispatch(ID3D12GraphicsCommandList* commandList, int x, int y, int z)
{
	commandList->Dispatch(x, y, z);
}
