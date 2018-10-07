#include "Entity.h"

void Entity::CalculateWorldMatrix()
{
	XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX scle = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX world = scle * rot * trans;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	XMStoreFloat4x4(&worldMatrix, world);
}

Entity::Entity()
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMVECTOR v = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR sc = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	XMStoreFloat3(&position, v);
	XMStoreFloat3(&scale, sc);
	XMStoreFloat3(&rotation, v);
}

void Entity::SetMesh(Mesh * m)
{
	mesh = m;
}

void Entity::SetMaterial(Material * mat)
{
	material = mat;
}

Material * Entity::GetMaterial()
{
	return material;
}

Mesh * Entity::GetMesh()
{
	return mesh;
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
	CalculateWorldMatrix();
	return worldMatrix;
}

XMFLOAT4X4 Entity::GetWorldMatrixTransposed()
{
	CalculateWorldMatrix();
	auto worldT = XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix));
	XMFLOAT4X4 worldTransposed;
	XMStoreFloat4x4(&worldTransposed, worldT);
	return worldTransposed;
}

XMFLOAT3 Entity::GetPosition()
{
	return position;
}

void Entity::SetPosition(const XMFLOAT3& pos)
{
	position = pos;
}

void Entity::SetScale(const XMFLOAT3 & scale)
{
	this->scale = scale;
}

Entity::~Entity()
{
}
