#pragma once
#include "stdafx.h"
#include "ComputeProcess.h"

class BlurFilter
{
	ComputeProcess* blurHorizontalCS;
	ComputeProcess* blurVerticalCS;
	ComputeCore* computeCore;
public:
	BlurFilter(ComputeCore* core);
	~BlurFilter();

	std::vector<float> CalcGaussWeights(float sigma);
	Texture* Blur(ID3D12GraphicsCommandList* commandList, std::vector<Texture*> textures, int blurRadius);
};

