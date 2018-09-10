#pragma once
#include "../stdafx.h"
#include "Vertex.h"

class Mesh
{
	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;
	UINT vBufferSize;
	UINT iBufferSize;
	ID3D12Device* device;
public:
	Mesh(ID3D12Device* device);
	Mesh(std::string objFile, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void Initialize(Vertex* vertices, UINT vertexCount, UINT* indices, UINT indexCount, ID3D12GraphicsCommandList* commandList);
	void CalculateTangents(Vertex* vertices, UINT vertexCount, UINT * indices, UINT indexCount);
	~Mesh();
};