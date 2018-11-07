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
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(2, range);
	rootParameters[1].InitAsConstants(4, 0);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(2, rootParameters);

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

void ComputeProcess::Dispatch(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetComputeRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineStateObject);
	commandList->Dispatch(8, 1, 1);
}
