#include "Entity.h"

Entity::Entity()
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMVECTOR pos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR vScale = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	XMStoreFloat3(&position, pos);
	XMStoreFloat3(&scale, vScale);
	XMStoreFloat3(&rotation, pos);
}

XMFLOAT4X4 Entity::GetWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX scle = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX world = scle * rot * trans;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	return worldMatrix;
}

void Entity::SetPosition(XMFLOAT3 position)
{
	this->position = position;
}

Entity::~Entity()
{
}
