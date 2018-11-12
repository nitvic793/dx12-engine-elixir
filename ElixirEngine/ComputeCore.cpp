#include "stdafx.h"
#include "ComputeCore.h"


void ComputeCore::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range[3];
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0);
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);

	CD3DX12_ROOT_PARAMETER rootParameters[4];
	rootParameters[0].InitAsDescriptorTable(1, &range[0]);
	rootParameters[1].InitAsDescriptorTable(1, &range[1]);
	rootParameters[2].InitAsConstants(32, 0);
	rootParameters[3].InitAsDescriptorTable(1, &range[2]);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
	StaticSamplers[0].Init(0, D3D12_FILTER_ANISOTROPIC);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(4, rootParameters);
	descRootSignature.NumStaticSamplers = 1;
	descRootSignature.pStaticSamplers = StaticSamplers;

	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	auto hr = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);

	device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
}

ComputeCore::ComputeCore(ID3D12Device* device)
{
	this->device = device;
	CreateRootSignature();
}

ID3D12RootSignature * ComputeCore::GetRootSignature()
{
	return rootSignature;
}

ID3D12Device * ComputeCore::GetDevice()
{
	return device;
}


ComputeCore::~ComputeCore()
{
	rootSignature->Release();
}
