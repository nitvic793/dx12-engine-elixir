#pragma once

#include <DirectXMath.h>
using namespace DirectX;

struct Vertex 
{
	Vertex(){}
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), uv(u, v) {}
	XMFLOAT3	pos;
	XMFLOAT2	uv;
	XMFLOAT3	normal;
	XMFLOAT3	tangent;
	float		padding[5];
};

struct VertexAnimated
{
	XMFLOAT3	pos;
	XMFLOAT2	uv;
	XMFLOAT3	normal;
	XMFLOAT3	tangent;
	XMFLOAT4	skinIndices;
	XMFLOAT4	skinWeights;
	float		padding;
};

struct VertexInstanceData
{
	XMFLOAT3 position;
};

struct ScreenQuadVertex
{
	XMFLOAT4 position;
	XMFLOAT2 uv;
};