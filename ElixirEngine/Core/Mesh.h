#pragma once
#include "../stdafx.h"
#include "Vertex.h"
#include <map>
#include "ConstantBuffer.h"
#define MaxBonesPerVertex 4

/// TODO:
/// Render animated mesh using AnimationDefaultVS first
/// Load Animations using aiScene and ogldev tutorial

struct BoneInfo
{
	XMFLOAT4X4 OffsetMatrix;
};

struct VertexBoneData
{
	uint32_t IDs[MaxBonesPerVertex];
	float Weights[MaxBonesPerVertex];

	void AddBoneData(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < MaxBonesPerVertex; i++) {
			if (Weights[i] == 0.0) {
				IDs[i] = boneID;
				Weights[i] = weight;
				return;
			}
		}
	}
};

struct BoneDescriptor
{
	std::map<std::string, uint32_t> boneMapping;
	std::vector<BoneInfo>			boneInfoList;
	std::vector<VertexBoneData>		bones;
};

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



struct BoneMesh
{
	ID3D12Resource*				boneVertexBuffer;
	ID3D12Resource*				vBufferUploadHeap;
	UINT						vBufferSize;
	D3D12_VERTEX_BUFFER_VIEW	vBufferView;
};

class Mesh
{
	ID3D12Device*							device;
	std::vector<SubMesh>					subMeshes;
	std::vector<BoneMesh>					boneMeshes;
	std::vector<BoneDescriptor>				boneDescriptors;
	std::vector<PerArmatureConstantBuffer>	boneCBs;
	BoundingSphere							boundingSphere;
	BoundingOrientedBox						boundingBox;
	bool									mIsAnimated;

public:
	Mesh(ID3D12Device* device);
	Mesh(std::string objFile, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	Mesh(ID3D12Device* device, int subMeshCount, bool hasBones = false);

	void Initialize(UINT meshIndex, Vertex* vertices, UINT vertexCount, UINT* indices, UINT indexCount, ID3D12GraphicsCommandList* commandList);
	void InitializeBoneWeights(UINT meshIndex, BoneDescriptor boneData, ID3D12GraphicsCommandList* commandList);
	void CalculateTangents(Vertex* vertices, UINT vertexCount, UINT * indices, UINT indexCount);

	const PerArmatureConstantBuffer GetArmatureCB(UINT index);
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(UINT index);
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBoneBufferView(UINT index);
	const D3D12_INDEX_BUFFER_VIEW&	GetIndexBufferView(UINT index);
	const UINT&						GetIndexCount(UINT index);
	const UINT						GetSubMeshCount();

	const BoundingSphere&			GetBoundingSphere();
	const BoundingOrientedBox&		GetBoundingOrientedBox();
	const bool						IsAnimated();
	~Mesh();
};