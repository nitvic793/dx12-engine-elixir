#include "stdafx.h"
#include "ComputeProcess.h"


void ComputeProcess::CreatePSO()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = rootSignature;
	computePsoDesc.CS = ShaderManager::LoadShader(shaderFileName.c_str());

	device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&pipelineStateObject));
	pipelineStateObject->SetName(shaderFileName.c_str());
}

void ComputeProcess::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range[2];
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, &range[0]);
	rootParameters[1].InitAsDescriptorTable(1, &range[1]);
	rootParameters[2].InitAsConstants(4, 0);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
	StaticSamplers[0].Init(0, D3D12_FILTER_ANISOTROPIC);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(3, rootParameters);
	descRootSignature.NumStaticSamplers = 1;
	descRootSignature.pStaticSamplers = StaticSamplers;

	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	auto hr = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);

	device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
}

ComputeProcess::ComputeProcess(ID3D12Device* device, std::wstring filename) :
	shaderFileName(filename)
{
	this->device = device;
	CreateRootSignature();
	CreatePSO();
}


ComputeProcess::~ComputeProcess()
{
	if (rootSignature)rootSignature->Release();
	if (pipelineStateObject)pipelineStateObject->Release();
}

void ComputeProcess::SetShader(ID3D12GraphicsCommandList * commandList)
{
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

void ComputeProcess::Dispatch(ID3D12GraphicsCommandList* commandList, int x, int y, int z)
{
	commandList->Dispatch(x, y, z);
}
