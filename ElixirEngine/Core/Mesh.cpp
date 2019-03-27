#include "Mesh.h"
#include <fstream>
#include <vector>
#include "DirectXMesh.h"
#include "../Utility.h"
#include <queue>
#include <stack>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace ogldev;

Mesh::Mesh(ID3D12Device * device)
{
	this->device = device;
}

Mesh::Mesh(std::string objFile, ID3D12Device * device, ID3D12GraphicsCommandList* commandList)
{
	subMeshes.resize(1);
	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts

	std::vector<Vertex> vertices;           // Verts we're assembling
	std::vector<UINT> indexVals;           // Indices of these verts

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objFile.c_str());
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		bool hasNormals = true;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			int fv = shapes[s].mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				// access to vertex
				Vertex vertex;
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				vertex.pos = XMFLOAT3(vx, vy, vz);
				positions.push_back(vertex.pos);

				if (idx.normal_index != -1)
				{
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					vertex.normal = XMFLOAT3(nx, ny, nz);
				}
				else
				{
					hasNormals = false;
				}

				tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
				vertex.uv = XMFLOAT2(tx, ty);
				vertices.push_back(vertex);

				indexVals.push_back((UINT)index_offset + (UINT)v);
				// Optional: vertex colors
				// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}

		if (!hasNormals)
		{
			XMFLOAT3 *normals = new XMFLOAT3[positions.size()];
			ComputeNormals(indexVals.data(), shapes[s].mesh.num_face_vertices.size(), positions.data(), positions.size(), CNORM_WEIGHT_BY_AREA, normals);
			for (uint32_t i = 0; i < positions.size(); ++i)
			{
				vertices[i].normal = normals[i];
			}

			delete[] normals;
		}
	}

	BoundingOrientedBox::CreateFromPoints(boundingBox, positions.size(), positions.data(), sizeof(XMFLOAT3));
	BoundingSphere::CreateFromPoints(boundingSphere, positions.size(), positions.data(), sizeof(XMFLOAT3));
	this->device = device;
	Initialize(0, vertices.data(), (UINT)vertices.size(), indexVals.data(), (UINT)indexVals.size(), commandList);
}

Mesh::Mesh(ID3D12Device * device, int subMeshCount, bool hasBones, const aiScene* scene) 
{
	this->device = device;
	subMeshes.resize(subMeshCount);
	if (hasBones)
	{
		mIsAnimated = true;
		boneMeshes.resize(subMeshCount);
		boneDescriptors.resize(subMeshCount);
		boneCBs.resize(subMeshCount);
	}
}


void Mesh::Initialize(UINT meshIndex, Vertex * vertices, UINT vertexCount, UINT * indices, UINT indexCount, ID3D12GraphicsCommandList * commandList)
{
	SubMesh subMesh;
	subMesh.indexCount = indexCount;
	CalculateTangents(vertices, vertexCount, indices, indexCount);
	subMesh.vBufferSize = sizeof(Vertex) * vertexCount;

	// create default heap
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(subMesh.vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&subMesh.vertexBuffer));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	subMesh.vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// create upload heap

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(subMesh.vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&subMesh.vBufferUploadHeap));
	subMesh.vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vertices); // pointer to our vertex array
	vertexData.RowPitch = subMesh.vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = subMesh.vBufferSize; // also the size of our triangle vertex data

	UpdateSubresources(commandList, subMesh.vertexBuffer, subMesh.vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(subMesh.vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	subMesh.iBufferSize = sizeof(UINT) * indexCount;

	// create default heap to hold index buffer
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(subMesh.iBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&subMesh.indexBuffer));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	subMesh.indexBuffer->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(subMesh.vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&subMesh.iBufferUploadHeap));
	subMesh.iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(indices); // pointer to our index array
	indexData.RowPitch = subMesh.iBufferSize; // size of all our index buffer
	indexData.SlicePitch = subMesh.iBufferSize; // also the size of our index buffer

										// we are now creating a command with the command list to copy the data from
										// the upload heap to the default heap
	UpdateSubresources(commandList, subMesh.indexBuffer, subMesh.iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(subMesh.indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	subMesh.vBufferView.BufferLocation = subMesh.vertexBuffer->GetGPUVirtualAddress();
	subMesh.vBufferView.StrideInBytes = sizeof(Vertex);
	subMesh.vBufferView.SizeInBytes = subMesh.vBufferSize;

	subMesh.iBufferView.BufferLocation = subMesh.indexBuffer->GetGPUVirtualAddress();
	subMesh.iBufferView.Format = DXGI_FORMAT_R32_UINT;
	subMesh.iBufferView.SizeInBytes = subMesh.iBufferSize;

	subMesh.vertices.assign(vertices, vertices + vertexCount);
	subMesh.indices.assign(indices, indices + indexCount);
	subMeshes[meshIndex] = subMesh;
}

void Mesh::InitializeBoneWeights(UINT meshIndex, BoneDescriptor boneData, ID3D12GraphicsCommandList * commandList)
{
	BoneMesh boneMesh;
	boneMesh.vBufferSize = sizeof(VertexBoneData) * (UINT)boneData.bones.size();

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(boneMesh.vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&boneMesh.boneVertexBuffer));

	boneMesh.boneVertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(boneMesh.vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&boneMesh.vBufferUploadHeap));

	boneMesh.vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(boneData.bones.data());
	vertexData.RowPitch = boneMesh.vBufferSize;
	vertexData.SlicePitch = boneMesh.vBufferSize;

	UpdateSubresources(commandList, boneMesh.boneVertexBuffer, boneMesh.vBufferUploadHeap, 0, 0, 1, &vertexData);

	boneMesh.vBufferView.BufferLocation = boneMesh.boneVertexBuffer->GetGPUVirtualAddress();
	boneMesh.vBufferView.StrideInBytes = sizeof(VertexBoneData);
	boneMesh.vBufferView.SizeInBytes = boneMesh.vBufferSize;

	PerArmatureConstantBuffer cb = {};

	boneCBs[meshIndex] = cb;
	boneMeshes[meshIndex] = boneMesh;
	boneDescriptors[meshIndex] = boneData;
	BoneTransform(0, 15.8f, 0);
}

void Mesh::CalculateTangents(Vertex * vertices, UINT vertexCount, UINT * indices, UINT indexCount)
{
	XMFLOAT3 *tan1 = new XMFLOAT3[vertexCount * 2];
	XMFLOAT3 *tan2 = tan1 + vertexCount;
	ZeroMemory(tan1, vertexCount * sizeof(XMFLOAT3) * 2);
	int triangleCount = indexCount / 3;
	for (UINT i = 0; i < indexCount; i += 3)
	{
		int i1 = indices[i];
		int i2 = indices[i + 2];
		int i3 = indices[i + 1];
		auto v1 = vertices[i1].pos;
		auto v2 = vertices[i2].pos;
		auto v3 = vertices[i3].pos;

		auto w1 = vertices[i1].uv;
		auto w2 = vertices[i2].uv;
		auto w3 = vertices[i3].uv;

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;
		float r = 1.0F / (s1 * t2 - s2 * t1);

		XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);

		XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		XMStoreFloat3(&tan1[i1], XMLoadFloat3(&tan1[i1]) + XMLoadFloat3(&sdir));
		XMStoreFloat3(&tan1[i2], XMLoadFloat3(&tan1[i2]) + XMLoadFloat3(&sdir));
		XMStoreFloat3(&tan1[i3], XMLoadFloat3(&tan1[i3]) + XMLoadFloat3(&sdir));

		XMStoreFloat3(&tan2[i1], XMLoadFloat3(&tan2[i1]) + XMLoadFloat3(&tdir));
		XMStoreFloat3(&tan2[i2], XMLoadFloat3(&tan2[i2]) + XMLoadFloat3(&tdir));
		XMStoreFloat3(&tan2[i3], XMLoadFloat3(&tan2[i3]) + XMLoadFloat3(&tdir));
	}

	for (UINT a = 0; a < vertexCount; a++)
	{
		auto n = vertices[a].normal;
		auto t = tan1[a];

		// Gram-Schmidt orthogonalize
		auto dot = XMVector3Dot(XMLoadFloat3(&n), XMLoadFloat3(&t));
		XMStoreFloat3(&vertices[a].tangent, XMVector3Normalize(XMLoadFloat3(&t) - XMLoadFloat3(&n)* dot));

		// Calculate handedness
		/*tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;*/
	}

	delete[] tan1;
}

void Mesh::BoneTransform(UINT meshIndex, float totalTime, UINT animationIndex)
{
	float TicksPerSecond = (float)(Animations.Animations[animationIndex].TicksPerSecond != 0 ? Animations.Animations[animationIndex].TicksPerSecond : 25.0f);
	float TimeInTicks = totalTime * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)Animations.Animations[animationIndex].Duration);

	ReadNodeHeirarchy(AnimationTime, animationIndex);

	for (uint32_t i = 0; i < boneDescriptors[meshIndex].boneInfoList.size(); i++)
	{
		XMFLOAT4X4 finalTransform;
		XMStoreFloat4x4(&finalTransform, XMMatrixTranspose(XMLoadFloat4x4(&boneDescriptors[meshIndex].boneInfoList[i].FinalTransform)));
		boneCBs[meshIndex].bones[i] = finalTransform;
	}
}

uint32_t FindPosition(float AnimationTime, AnimationChannel* channel)
{
	for (uint32_t i = 0; i < channel->PositionKeys.size() - 1; i++) {
		if (AnimationTime < (float)channel->PositionKeys[i + 1].Time) {
			return i;
		}
	}

	assert(0);
	return 0;
}

uint32_t FindScaling(float AnimationTime, AnimationChannel* channel)
{
	for (uint32_t i = 0; i < channel->ScalingKeys.size() - 1; i++) {
		if (AnimationTime < (float)channel->ScalingKeys[i + 1].Time) {
			return i;
		}
	}

	assert(0);
	return 0;
}

uint32_t FindRotation(float AnimationTime, AnimationChannel* channel)
{
	for (uint32_t i = 0; i < channel->RotationKeys.size() - 1; i++) {
		if (AnimationTime < (float)channel->RotationKeys[i + 1].Time) {
			return i;
		}
	}

	assert(0);
	return 0;
}

XMFLOAT3 InterpolatePosition(float animTime, AnimationChannel* channel)
{
	auto outFloat3 = XMFLOAT3();
	auto Out = XMVectorSet(0, 0, 0, 0);
	if (channel->PositionKeys.size() == 1) {
		//Out = XMLoadFloat3(&channel->PositionKeys[0].Value);
		return channel->PositionKeys[0].Value;
	}

	uint32_t PositionIndex = FindPosition(animTime, channel);
	uint32_t NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < channel->PositionKeys.size());

	float DeltaTime = (float)(channel->PositionKeys[NextPositionIndex].Time - channel->PositionKeys[PositionIndex].Time);
	float Factor = (animTime - (float)channel->PositionKeys[PositionIndex].Time) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);

	auto Start = XMLoadFloat3(&channel->PositionKeys[PositionIndex].Value);
	auto End = XMLoadFloat3(&channel->PositionKeys[NextPositionIndex].Value);
	auto Delta = End - Start;
	Out = Start + Factor * Delta;
	XMStoreFloat3(&outFloat3, Out);
	return outFloat3;
}


XMFLOAT3 InterpolateScaling(float animTime, AnimationChannel* channel)
{
	auto outFloat3 = XMFLOAT3();
	auto Out = XMVectorSet(0, 0, 0, 0);
	if (channel->ScalingKeys.size() == 1) {
		//Out = XMLoadFloat3(&channel->ScalingKeys[0].Value);
		return channel->ScalingKeys[0].Value;
	}

	uint32_t ScaleIndex = FindScaling(animTime, channel);
	uint32_t NextScaleIndex = (ScaleIndex + 1);
	assert(NextScaleIndex < channel->ScalingKeys.size());

	float DeltaTime = (float)(channel->ScalingKeys[NextScaleIndex].Time - channel->ScalingKeys[ScaleIndex].Time);
	float Factor = (animTime - (float)channel->ScalingKeys[ScaleIndex].Time) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);

	auto Start = XMLoadFloat3(&channel->ScalingKeys[ScaleIndex].Value);
	auto End = XMLoadFloat3(&channel->ScalingKeys[NextScaleIndex].Value);
	auto Delta = End - Start;
	Out = Start + Factor * Delta;
	XMStoreFloat3(&outFloat3, Out);
	return outFloat3;
}

XMFLOAT4 InterpolateRotation(float animTime, AnimationChannel* channel)
{
	auto outFloat4 = XMFLOAT4();
	auto Out = XMVectorSet(0, 0, 0, 0);
	if (channel->RotationKeys.size() == 1) {
		//Out = XMLoadFloat4(&channel->RotationKeys[0].Value);
		return channel->RotationKeys[0].Value;
	}

	uint32_t RotationIndex = FindRotation(animTime, channel);
	uint32_t NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < channel->RotationKeys.size());

	float DeltaTime = (float)(channel->RotationKeys[NextRotationIndex].Time - channel->RotationKeys[RotationIndex].Time);
	float Factor = (animTime - (float)channel->RotationKeys[RotationIndex].Time) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);

	auto StartRotationQ = XMLoadFloat4(&channel->RotationKeys[RotationIndex].Value);
	auto EndRotationQ = XMLoadFloat4(&channel->RotationKeys[NextRotationIndex].Value);
	Out = XMQuaternionSlerp((StartRotationQ), (EndRotationQ), Factor);
	Out = XMQuaternionNormalize(Out);
	XMStoreFloat4(&outFloat4, Out);
	return outFloat4;
}

void Mesh::ReadNodeHeirarchy(float AnimationTime, UINT animationIndex)
{
	auto& nodes = Animations.NodeHeirarchy;
	auto rootNode = Animations.RootNode;
	std::stack<std::string> nodeQueue;
	std::stack<XMFLOAT4X4> transformationQueue;
	auto globalInverse = XMLoadFloat4x4(&Animations.GlobalInverseTransform);
	auto rootTransform = XMMatrixIdentity();
	XMFLOAT4X4 identity;
	XMFLOAT4X4 globalFloat4x4;
	XMStoreFloat4x4(&identity, rootTransform);
	nodeQueue.push(rootNode);
	transformationQueue.push(identity);

	while (!nodeQueue.empty())
	{
		auto node = nodeQueue.top();
		auto parentTransformation = XMLoadFloat4x4(&transformationQueue.top());
		auto nodeTransformation = XMLoadFloat4x4(&Animations.NodeTransformsMap[node]);

		nodeQueue.pop();
		transformationQueue.pop();

		auto anim = Animations.GetChannel(animationIndex, node);
		if (anim != nullptr)
		{
			auto s = InterpolateScaling(AnimationTime, anim);
			auto scaling = XMMatrixScaling(s.x, s.y, s.z);

			auto r = InterpolateRotation(AnimationTime, anim);
			auto rotation = XMMatrixRotationQuaternion(XMVectorSet(r.y, r.z, r.w, r.x));

			auto t = InterpolatePosition(AnimationTime, anim);
			auto translation = XMMatrixTranslation(t.x, t.y, t.z);

			nodeTransformation += scaling * rotation * translation;
		}

		auto globalTransformation = nodeTransformation * parentTransformation;
		if (boneDescriptors[0].boneMapping.find(node) != boneDescriptors[0].boneMapping.end())
		{
			uint32_t BoneIndex = boneDescriptors[0].boneMapping[node];
			auto finalTransform = XMMatrixTranspose(OGLtoXM(boneDescriptors[0].boneInfoList[BoneIndex].Offset)) * globalTransformation * globalInverse;
			XMStoreFloat4x4(&boneDescriptors[0].boneInfoList[BoneIndex].FinalTransform, finalTransform);
		}

		auto children = Animations.NodeHeirarchy[node];
		for (int i = (int)children.size() - 1; i >= 0; --i)
		{
			XMStoreFloat4x4(&globalFloat4x4, globalTransformation);
			nodeQueue.push(children[i]);
			transformationQueue.push(globalFloat4x4);
		}
	}
}

const PerArmatureConstantBuffer Mesh::GetArmatureCB(UINT index)
{
	return boneCBs[index];
}

const D3D12_VERTEX_BUFFER_VIEW & Mesh::GetVertexBufferView(UINT index)
{
	return subMeshes[index].vBufferView;
}

const D3D12_VERTEX_BUFFER_VIEW& Mesh::GetVertexBoneBufferView(UINT index)
{
	return boneMeshes[index].vBufferView;
}

const D3D12_INDEX_BUFFER_VIEW & Mesh::GetIndexBufferView(UINT index)
{
	return subMeshes[index].iBufferView;
}

const UINT & Mesh::GetIndexCount(UINT index)
{
	return subMeshes[index].indexCount;
}

const UINT Mesh::GetSubMeshCount()
{
	return (UINT)subMeshes.size();
}

const BoundingSphere & Mesh::GetBoundingSphere()
{
	return boundingSphere;
}

const BoundingOrientedBox & Mesh::GetBoundingOrientedBox()
{
	return boundingBox;
}

const bool Mesh::IsAnimated()
{
	return mIsAnimated;
}

Mesh::~Mesh()
{
	for (auto sm : subMeshes)
	{
		if (sm.vertexBuffer)sm.vertexBuffer->Release();
		if (sm.indexBuffer)sm.indexBuffer->Release();
		if (sm.iBufferUploadHeap)sm.iBufferUploadHeap->Release(); //Most probably should have one upload heap for all meshes
		if (sm.vBufferUploadHeap)sm.vBufferUploadHeap->Release();
	}

	for (auto bm : boneMeshes)
	{
		if (bm.boneVertexBuffer)bm.boneVertexBuffer->Release();
		if (bm.vBufferUploadHeap)bm.vBufferUploadHeap->Release();
	}
}
