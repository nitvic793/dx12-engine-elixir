#pragma once

#include <DirectXMath.h>
using namespace DirectX;

struct Vertex 
{
	Vertex(){}
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), uv(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	float padding[5];
};

struct VertexInstanceData
{
	XMFLOAT3 position;
};

struct ScreenQuadVertex
{
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT2 uv;
};