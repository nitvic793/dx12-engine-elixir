#include "Entity.h"

Entity::Entity()
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMVECTOR v = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR sc = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	XMStoreFloat3(&position, v);
	XMStoreFloat3(&scale, sc);
	XMStoreFloat3(&rotation, v);
}

XMFLOAT4X4 Entity::GetWorldViewProjectionTransposed(XMFLOAT4X4 projection, XMFLOAT4X4 view)
{
	XMMATRIX viewMat = XMLoadFloat4x4(&view); // load view matrix
	XMMATRIX projMat = XMLoadFloat4x4(&projection); // load projection matrix
	XMMATRIX wvpMat = XMLoadFloat4x4(&GetWorldMatrix()) * viewMat * projMat; // create wvp matrix
	XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	XMFLOAT4X4 wvp;
	XMStoreFloat4x4(&wvp, transposed); // store transposed wvp matrix in constant buffer
	return wvp;
}

XMFLOAT4X4 Entity::GetWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX scle = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX world = scle * rot * trans;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	XMStoreFloat4x4(&worldMatrix, world);
	return worldMatrix;
}

XMFLOAT3 Entity::GetPosition()
{
	return position;
}

void Entity::SetPosition(const XMFLOAT3& pos)
{
	position = pos;
}

Entity::~Entity()
{
}
