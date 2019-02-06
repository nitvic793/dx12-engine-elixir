#include "stdafx.h"
#include "MeshInstanceGroupEntity.h"
#include "Core/ConstantBuffer.h"

MeshInstanceGroupEntity::MeshInstanceGroupEntity()
{
}

MeshInstanceGroupEntity::MeshInstanceGroupEntity(std::vector<HashID> meshes, std::vector<HashID> materials, std::vector<XMFLOAT3> positions, ID3D12Device* device,
	ID3D12GraphicsCommandList* clist)
{
	castsShadow = true;
	this->meshes = meshes;
	this->materials = materials;
	InitializeBuffer(positions, device, clist);
}

void MeshInstanceGroupEntity::InitializeBuffer(std::vector<XMFLOAT3> positions, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	auto vBufferSize = positions.size() * sizeof(InstanceWorldBuffer);
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&instanceBuffer));

	instanceBuffer->SetName(L"Instance Buffer Resource Heap");

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap));
	uploadHeap->SetName(L"Instance Buffer Upload Resource Heap");

	for (auto p : positions)
	{
		XMFLOAT4X4 transform;
		auto translation = XMMatrixTranslationFromVector(XMLoadFloat3(&p));
		auto scale = XMMatrixScalingFromVector(XMVectorSet(1, 1, 1, 0));
		auto rotation = XMMatrixRotationRollPitchYawFromVector(XMVectorSet(0, 0, 0, 0));
		auto world = rotation * scale * translation;
		XMStoreFloat4x4(&transform, XMMatrixTranspose(world));
		transforms.push_back(transform);
	}

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(transforms.data());
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	UpdateSubresources(commandList, instanceBuffer, uploadHeap, 0, 0, 1, &vertexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(instanceBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	vBufferView.BufferLocation = instanceBuffer->GetGPUVirtualAddress();
	vBufferView.StrideInBytes = sizeof(InstanceWorldBuffer);
	vBufferView.SizeInBytes = (uint32_t)vBufferSize;

}

const std::vector<HashID>& MeshInstanceGroupEntity::GetMeshIDs()
{
	return meshes;
}

const std::vector<HashID>& MeshInstanceGroupEntity::GetMaterialIDs()
{
	return materials;
}

const uint32_t MeshInstanceGroupEntity::GetInstanceCount()
{
	return (uint32_t)transforms.size();
}

const D3D12_VERTEX_BUFFER_VIEW & MeshInstanceGroupEntity::GetInstanceBufferView()
{
	return vBufferView;
}

const bool & MeshInstanceGroupEntity::CastsShadow()
{
	return castsShadow;
}

void MeshInstanceGroupEntity::SetCastsShadow(bool enable)
{
	castsShadow = enable;
}

MeshInstanceGroupEntity::~MeshInstanceGroupEntity()
{
	uploadHeap->Release();
	instanceBuffer->Release();
}
