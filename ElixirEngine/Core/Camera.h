#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT4X4 reflectionMatrix;
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT3 tdirection;

	float rotationY;
	float rotationX;
	XMFLOAT3 currentUp;
public:
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetDirection();
	void RotateX(float x);
	void RotateY(float y);
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();
	void SetProjectionMatrix(float width, float height);
	virtual void Update(float deltaTime);
	Camera(float width, float height);
	XMFLOAT3 GetUp();
	~Camera();
};