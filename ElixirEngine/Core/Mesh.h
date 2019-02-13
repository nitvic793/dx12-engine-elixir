#pragma once
#include "../stdafx.h"
#include "Vertex.h"

struct SubMesh
{
	std::vector<Vertex> vertices;
	std::vector<UINT>	indices;

	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;
	ID3D12Resource* vBufferUploadHeap;
	ID3D12Resource* iBufferUploadHeap;

	UINT vBufferSize;
	UINT iBufferSize;
	UINT indexCount;

	D3D12_VERTEX_BUFFER_VIEW vBufferView;
	D3D12_INDEX_BUFFER_VIEW iBufferView;
};



class Mesh
{
	ID3D12Device* device;
	std::vector<SubMesh> subMeshes;
	BoundingSphere boundingSphere;
	BoundingOrientedBox boundingBox;
	bool mIsAnimated;

public:
	Mesh(ID3D12Device* device);
	Mesh(std::string objFile, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	Mesh(ID3D12Device* device, int subMeshCount);

	void Initialize(UINT meshIndex, Vertex* vertices, UINT vertexCount, UINT* indices, UINT indexCount, ID3D12GraphicsCommandList* commandList);
	void CalculateTangents(Vertex* vertices, UINT vertexCount, UINT * indices, UINT indexCount);

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(UINT index);
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(UINT index);
	const UINT& GetIndexCount(UINT index);
	const UINT  GetSubMeshCount();

	const BoundingSphere& GetBoundingSphere();
	const BoundingOrientedBox& GetBoundingOrientedBox();
	const bool IsAnimated();
	~Mesh();
};