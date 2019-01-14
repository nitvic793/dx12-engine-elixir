#pragma once
#include "../stdafx.h"
#include "Light.h"

static const int MaxPointLights = 16;
static const int MaxDirLights = 4;

struct ConstantBuffer 
{
	DirectX::XMFLOAT4X4 worldViewProjection;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 shadowView;
	DirectX::XMFLOAT4X4 shadowProjection;
};

struct PixelConstantBuffer
{
	DirectionalLight light[MaxDirLights];
	PointLight pointLight[MaxPointLights];
	XMFLOAT4X4 invProjView;
	XMFLOAT3 cameraPosition;
	uint32_t pointLightCount;
	uint32_t pointLightIndex;
	uint32_t dirLightCount;
	uint32_t dirLightIndex;
};

struct PointShadowBuffer
{
	DirectX::XMFLOAT4X4 viewProjection[6];
};

struct PerFrameConstantBuffer
{
	float nearZ;
	float farZ;
	XMFLOAT2 lightPerspective;
};
