#pragma once
#include "../stdafx.h"
#include "Light.h"

struct ConstantBuffer 
{
	DirectX::XMFLOAT4X4 worldViewProjection;
};

struct PixelConstantBuffer
{
	DirectionalLight light;
};
