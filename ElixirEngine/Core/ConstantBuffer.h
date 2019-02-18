#pragma once
#include "../stdafx.h"
#include "Light.h"

static const int MaxPointLights = 16;
static const int MaxDirLights = 4;
static const int MaxBones = 128;

struct ConstantBuffer 
{
	XMFLOAT4X4	worldViewProjection;
	XMFLOAT4X4	world;
	XMFLOAT4X4	view;
	XMFLOAT4X4	projection;
	XMFLOAT4X4	shadowView;
	XMFLOAT4X4	shadowProjection;
	XMFLOAT2	uvScale;
};

struct InstanceWorldBuffer
{
	DirectX::XMFLOAT4X4 worldInstance;
};

struct PixelConstantBuffer
{
	DirectionalLight	light[MaxDirLights];
	PointLight			pointLight[MaxPointLights];
	XMFLOAT4X4			invProjView;
	XMFLOAT3			cameraPosition;
	uint32_t			pointLightCount;
	uint32_t			pointLightIndex;
	uint32_t			dirLightCount;
	uint32_t			dirLightIndex;
};

struct DirShadowBuffer
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 shadowView;
	DirectX::XMFLOAT4X4 shadowProjection;
};

struct PointShadowBuffer
{
	DirectX::XMFLOAT4X4 viewProjection[6];
};

struct PerFrameConstantBuffer
{
	float		nearZ;
	float		farZ;
	XMFLOAT2	lightPerspective;
};

struct PerArmatureConstantBuffer
{
	XMFLOAT4X4 bones[MaxBones];
};
