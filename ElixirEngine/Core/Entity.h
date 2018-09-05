#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Entity
{
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMFLOAT4X4 worldMatrix;

public:
	Entity();
	XMFLOAT4X4 GetWorldMatrix();
	void SetPosition(XMFLOAT3 position);
	~Entity();
};