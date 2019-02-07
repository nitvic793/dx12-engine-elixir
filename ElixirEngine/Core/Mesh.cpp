#include "Mesh.h"
#include <fstream>
#include <vector>
#include "DirectXMesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>



////
/// END TODO
////


Mesh::Mesh(ID3D12Device * device)
{
	this->device = device;
}

Mesh::Mesh(std::string objFile, ID3D12Device * device, ID3D12GraphicsCommandList* commandList)
{
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

	//// File input object
	//std::ifstream obj(objFile);

	//// Check for successful open
	//if (!obj.is_open())
	//{
	//	return;
	//}

	//unsigned int vertCounter = 0;        // Count of vertices/indices
	//char chars[100];                     // String for line reading

	//									 // Still have data left?
	//while (obj.good())
	//{
	//	// Get the line (100 characters should be more than enough)
	//	obj.getline(chars, 100);

	//	// Check the type of line
	//	if (chars[0] == 'v' && chars[1] == 'n')
	//	{
	//		// Read the 3 numbers directly into an XMFLOAT3
	//		XMFLOAT3 norm;
	//		sscanf_s(
	//			chars,
	//			"vn %f %f %f",
	//			&norm.x, &norm.y, &norm.z);

	//		// Add to the list of normals
	//		normals.push_back(norm);
	//	}
	//	else if (chars[0] == 'v' && chars[1] == 't')
	//	{
	//		// Read the 2 numbers directly into an XMFLOAT2
	//		XMFLOAT2 uv;
	//		sscanf_s(
	//			chars,
	//			"vt %f %f",
	//			&uv.x, &uv.y);

	//		// Add to the list of uv's
	//		uvs.push_back(uv);
	//	}
	//	else if (chars[0] == 'v')
	//	{
	//		// Read the 3 numbers directly into an XMFLOAT3
	//		XMFLOAT3 pos;
	//		sscanf_s(
	//			chars,
	//			"v %f %f %f",
	//			&pos.x, &pos.y, &pos.z);

	//		// Add to the positions
	//		positions.push_back(pos);
	//	}
	//	else if (chars[0] == 'f')
	//	{
	//		// Read the face indices into an array
	//		unsigned int i[12];
	//		int facesRead = sscanf_s(
	//			chars,
	//			"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
	//			&i[0], &i[1], &i[2],
	//			&i[3], &i[4], &i[5],
	//			&i[6], &i[7], &i[8],
	//			&i[9], &i[10], &i[11]);

	//		// - Create the verts by looking up
	//		//    corresponding data from vectors
	//		// - OBJ File indices are 1-based, so
	//		//    they need to be adusted
	//		Vertex v1;
	//		v1.pos = positions[i[0] - 1];
	//		v1.uv = uvs[i[1] - 1];
	//		v1.normal = normals[i[2] - 1];

	//		Vertex v2;
	//		v2.pos = positions[i[3] - 1];
	//		v2.uv = uvs[i[4] - 1];
	//		v2.normal = normals[i[5] - 1];

	//		Vertex v3;
	//		v3.pos = positions[i[6] - 1];
	//		v3.uv = uvs[i[7] - 1];
	//		v3.normal = normals[i[8] - 1];

	//		// The model is most likely in a right-handed space,
	//		// especially if it came from Maya.  We want to convert
	//		// to a left-handed space for DirectX.  This means we 
	//		// need to:
	//		//  - Invert the Z position
	//		//  - Invert the normal's Z
	//		//  - Flip the winding order
	//		// We also need to flip the UV coordinate since DirectX
	//		// defines (0,0) as the top left of the texture, and many
	//		// 3D modeling packages use the bottom left as (0,0)

	//		// Flip the UV's since they're probably "upside down"
	//		v1.uv.y = 1.0f - v1.uv.y;
	//		v2.uv.y = 1.0f - v2.uv.y;
	//		v3.uv.y = 1.0f - v3.uv.y;

	//		// Flip Z (LH vs. RH)
	//		v1.pos.z *= -1.0f;
	//		v2.pos.z *= -1.0f;
	//		v3.pos.z *= -1.0f;

	//		// Flip normal Z
	//		v1.normal.z *= -1.0f;
	//		v2.normal.z *= -1.0f;
	//		v3.normal.z *= -1.0f;

	//		// Add the verts to the vector (flipping the winding order)
	//		verts.push_back(v1);
	//		verts.push_back(v3);
	//		verts.push_back(v2);

	//		// Add three more indices
	//		indices.push_back(vertCounter); vertCounter += 1;
	//		indices.push_back(vertCounter); vertCounter += 1;
	//		indices.push_back(vertCounter); vertCounter += 1;

	//		// Was there a 4th face?
	//		if (facesRead == 12)
	//		{
	//			// Make the last vertex
	//			Vertex v4;
	//			v4.pos = positions[i[9] - 1];
	//			v4.uv = uvs[i[10] - 1];
	//			v4.normal = normals[i[11] - 1];

	//			// Flip the UV, Z pos and normal
	//			v4.uv.y = 1.0f - v4.uv.y;
	//			v4.pos.z *= -1.0f;
	//			v4.normal.z *= -1.0f;

	//			// Add a whole triangle (flipping the winding order)
	//			verts.push_back(v1);
	//			verts.push_back(v4);
	//			verts.push_back(v3);

	//			// Add three more indices
	//			indices.push_back(vertCounter); vertCounter += 1;
	//			indices.push_back(vertCounter); vertCounter += 1;
	//			indices.push_back(vertCounter); vertCounter += 1;
	//		}
	//	}
	//}

	//// Close the file and create the actual buffers
	//obj.close();

	//Initialize(verts.data(), (UINT)verts.size(), indices.data(), (UINT)indices.size(), commandList);
	Initialize(vertices.data(), (UINT)vertices.size(), indexVals.data(), (UINT)indexVals.size(), commandList);
}

void Mesh::Initialize(Vertex* vertices, UINT vertexCount, UINT * indices, UINT indexCount, ID3D12GraphicsCommandList* commandList)
{
	this->indexCount = indexCount;
	CalculateTangents(vertices, vertexCount, indices, indexCount);
	vBufferSize = sizeof(Vertex) * vertexCount;

	// create default heap
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&vertexBuffer));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// create upload heap
	
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vertices); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

	UpdateSubresources(commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	iBufferSize = sizeof(UINT) * indexCount;

	// create default heap to hold index buffer
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&indexBuffer));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	indexBuffer->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(indices); // pointer to our index array
	indexData.RowPitch = iBufferSize; // size of all our index buffer
	indexData.SlicePitch = iBufferSize; // also the size of our index buffer

										// we are now creating a command with the command list to copy the data from
										// the upload heap to the default heap
	UpdateSubresources(commandList, indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	vBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vBufferView.StrideInBytes = sizeof(Vertex);
	vBufferView.SizeInBytes = vBufferSize;

	iBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	iBufferView.Format = DXGI_FORMAT_R32_UINT;
	iBufferView.SizeInBytes = iBufferSize;
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

const D3D12_VERTEX_BUFFER_VIEW& Mesh::GetVertexBufferView()
{
	return vBufferView;
}

const D3D12_INDEX_BUFFER_VIEW& Mesh::GetIndexBufferView()
{
	return iBufferView;
}

const UINT& Mesh::GetIndexCount()
{
	return indexCount;
}

const BoundingSphere & Mesh::GetBoundingSphere()
{
	return boundingSphere;
}

const BoundingOrientedBox & Mesh::GetBoundingOrientedBox()
{
	return boundingBox;
}

Mesh::~Mesh()
{
	vertexBuffer->Release();
	indexBuffer->Release();

	iBufferUploadHeap->Release(); //Most probably should have one upload heap for all meshes
	vBufferUploadHeap->Release();
}
