#pragma once
#include "../stdafx.h"
#include "Mesh.h"

class Entity
{
protected:
	XMFLOAT4X4 worldMatrix;
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT3 rotation;
	Mesh* mesh;
	void CalculateWorldMatrix();
public:
	Entity();
	void SetMesh(Mesh* m);
	Mesh* GetMesh();
	XMFLOAT4X4 GetWorldViewProjectionTransposed(XMFLOAT4X4 projection, XMFLOAT4X4 view);
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT4X4 GetWorldMatrixTransposed();
	XMFLOAT3 GetPosition();
	void SetPosition(const XMFLOAT3& pos);
	~Entity();
};