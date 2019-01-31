#pragma once

#include "Core/Mesh.h"
#include "Material.h"

class MeshInstanceGroupEntity 
{
	std::vector<HashID> meshes;
	std::vector<HashID> materials;
	std::vector<XMFLOAT4X4> transforms;
	ID3D12Resource* instanceBuffer;
	ID3D12Resource* uploadHeap;
	D3D12_VERTEX_BUFFER_VIEW vBufferView;

public:
	MeshInstanceGroupEntity();
	MeshInstanceGroupEntity(
		std::vector<HashID> meshes, 
		std::vector<HashID> materials,
		std::vector<XMFLOAT3> positions,
		ID3D12Device* device,
		ID3D12GraphicsCommandList* clist
	);
	void InitializeBuffer(std::vector<XMFLOAT3> positions, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	const std::vector<HashID>& GetMeshIDs();
	const std::vector<HashID>& GetMaterialIDs();
	const size_t GetInstanceCount();
	const D3D12_VERTEX_BUFFER_VIEW& GetInstanceBufferView();
	~MeshInstanceGroupEntity();
};


