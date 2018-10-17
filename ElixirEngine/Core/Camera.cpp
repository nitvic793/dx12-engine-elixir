#include "Camera.h"
#include <Windows.h>

XMFLOAT3 Camera::GetPosition()
{
	return position;
}

void Camera::Update(float deltaTime)
{
	float speed = 10.f;
	XMVECTOR pos = XMVectorSet(position.x, position.y, position.z, 0);
	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 0);
	dir = XMVector3Rotate(dir, DirectX::XMLoadFloat4(&rotation));
	XMVECTOR up = XMVectorSet(0, 1, 0, 0); // Y is up!

	if (GetAsyncKeyState('W') & 0x8000)
	{
		pos = pos + dir * speed * deltaTime;;
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		pos = pos - dir * speed * deltaTime;;
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		auto leftDir = XMVector3Cross(dir, up);
		pos = pos + leftDir * speed * deltaTime;;
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		auto rightDir = XMVector3Cross(-dir, up);
		pos = pos + rightDir * speed * deltaTime;;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		pos = pos + XMVectorSet(0, speed * deltaTime, 0, 0);
	}
	if (GetAsyncKeyState('X') & 0x8000)
	{
		pos = pos + XMVectorSet(0, -speed * deltaTime, 0, 0);
	}

	//dir = XMVector3Rotate(pos, XMLoadFloat4(&rotation));
	XMStoreFloat3(&position, pos);
}

void Camera::SetPosition(const XMFLOAT3& pos)
{
	position = pos;
}

const XMFLOAT4X4& Camera::GetViewMatrix()
{
	auto rotQuaternion = XMQuaternionRotationRollPitchYaw(rotationX, rotationY, 0);
	XMVECTOR pos = XMVectorSet(position.x, position.y, position.z, 0);
	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 0);
	//XMVECTOR dir = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&rotation));
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	dir = XMVector3Rotate(dir, rotQuaternion);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));
	XMStoreFloat4x4(&viewMatrix, V);
	return viewMatrix;
}

const XMFLOAT4X4& Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

const XMFLOAT4X4 & Camera::GetInverseProjectionViewMatrix()
{
	auto proj = XMLoadFloat4x4(&projectionMatrix);
	auto view = XMLoadFloat4x4(&viewMatrix);
	auto invProjView = XMMatrixInverse(nullptr, proj * view);
	XMStoreFloat4x4(&inverseProjectionView, invProjView);
	return inverseProjectionView;
}

XMFLOAT4X4 Camera::GetViewMatrixTransposed()
{
	auto mat = XMLoadFloat4x4(&viewMatrix);
	XMFLOAT4X4 viewT;
	XMStoreFloat4x4(&viewT, XMMatrixTranspose(mat));
	return viewT;
}

XMFLOAT4X4 Camera::GetProjectionMatrixTransposed()
{
	auto mat = XMLoadFloat4x4(&projectionMatrix);
	XMFLOAT4X4 projectionT;
	XMStoreFloat4x4(&projectionT, XMMatrixTranspose(mat));
	return projectionT;
}

void Camera::Rotate(float x, float y)
{
	rotationX += x;// *XM_PIDIV2;
	rotationY += y;// *XM_PIDIV2;

	rotationX = max(min(rotationX, XM_PIDIV2), -XM_PIDIV2);

	XMStoreFloat4(&rotation, XMQuaternionRotationRollPitchYaw(rotationX, rotationY, 0));
}

void Camera::SetProjectionMatrix(float width, float height)
{
	float aspectRatio = width / height;
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * XM_PI,
		aspectRatio,
		0.1f,
		1000.0f);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
	XMStoreFloat4x4(&projectionMatrix, P);
}

Camera::Camera(float width, float height)
{
	float aspectRatio = width / height;
	XMStoreFloat4(&rotation, XMQuaternionIdentity());
	position = XMFLOAT3(0.f, 1.f, -5.f);
	direction = XMFLOAT3(0.f, 0.f, 1.f);
	rotationX = rotationY = 0.f;

	XMVECTOR pos = XMVectorSet(0, -1, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * XM_PI,		// Field of View Angle
		aspectRatio,		// Aspect ratio
		0.1f,						// Near clip plane distance
		1000.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
	XMStoreFloat4x4(&projectionMatrix, P);
}

Camera::~Camera()
{
}
