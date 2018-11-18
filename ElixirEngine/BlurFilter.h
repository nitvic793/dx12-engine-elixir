#pragma once
#include "stdafx.h"
#include "ComputeProcess.h"
#include "TexturePool.h"

class BlurFilter
{
	ComputeProcess* blurHorizontalCS;
	ComputeProcess* blurVerticalCS;
	ComputeCore* computeCore;
public:
	BlurFilter(ComputeCore* core);
	~BlurFilter();

	std::vector<float> CalcGaussWeights(float sigma);
	Texture* Apply(ID3D12GraphicsCommandList* commandList, Texture* inputSRV, TexturePool* texturePool, int blurRadius, float focusPlane, float focalLength);
};

