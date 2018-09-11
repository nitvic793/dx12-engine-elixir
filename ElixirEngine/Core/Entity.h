#pragma once
#include "../stdafx.h"

class Entity
{
protected:
	XMFLOAT4X4 worldMatrix;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT3 rotation;

public:
	Entity();
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT3 GetPosition();
	void SetPosition(XMFLOAT3 pos);
	~Entity();
};