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
	void Blur(ID3D12GraphicsCommandList* commandList, Texture* outputUAV, Texture* inputSRV, int blurRadius);
};

