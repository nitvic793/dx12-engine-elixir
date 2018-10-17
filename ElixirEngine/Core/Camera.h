#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT4X4 inverseProjectionView;
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT4 rotation;
	float rotationX;
	float rotationY;
public:
	XMFLOAT3 GetPosition();
	void Update(float deltaTime);
	void SetPosition(const XMFLOAT3& pos);
	const XMFLOAT4X4& GetViewMatrix();
	const XMFLOAT4X4& GetProjectionMatrix();
	const XMFLOAT4X4& GetInverseProjectionViewMatrix();

	XMFLOAT4X4 GetViewMatrixTransposed();
	XMFLOAT4X4 GetProjectionMatrixTransposed();

	void Rotate(float x, float y);
	void SetProjectionMatrix(float width, float height);
	Camera(float width, float height);
	~Camera();
};