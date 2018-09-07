#pragma once

#include <DirectXMath.h>
using namespace DirectX;

struct Vertex {
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), uv(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};