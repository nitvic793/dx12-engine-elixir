#pragma once

#include <DirectXMath.h>
using namespace DirectX;

struct Vertex {
	Vertex(float x, float y, float z, float r, float g, float b, float a) : 
		pos(x, y, z), 
		color(r, g, b, z) {}
	XMFLOAT3 pos;
	XMFLOAT4 color;
};