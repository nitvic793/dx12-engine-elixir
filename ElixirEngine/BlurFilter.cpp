#include "stdafx.h"
#include "BlurFilter.h"


BlurFilter::BlurFilter(ComputeCore* core)
{
	blurHorizontalCS = new ComputeProcess(core, L"BlurHorizontalCS.cso");
	blurVerticalCS = new ComputeProcess(core, L"BlurVerticalCS.cso");
}


BlurFilter::~BlurFilter()
{
	delete blurHorizontalCS;
	delete blurVerticalCS;
}

/**
Credits: Frank D Luna
URL: https://github.com/d3dcoder/d3d12book/blob/master/Chapter%2013%20The%20Compute%20Shader/Blur/BlurFilter.cpp
**/
std::vector<float> BlurFilter::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;
		weights[i + blurRadius] = expf(-x * x / twoSigma2);
		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

//TODO: Change input parameters since blur radius is being recalculated
Texture* BlurFilter::Apply(ID3D12GraphicsCommandList* commandList, Texture* inputSRV, TexturePool* texturePool, int blurRadius, float focusPlane, float focalLength)
{
	auto input = inputSRV;
	auto outputUAV = texturePool->GetUAV(0);
	auto weights = CalcGaussWeights(2.5f);
	blurRadius = (int)weights.size() / 2;
	UINT height = 720;
	UINT width = 1280;
	for (int i = 0; i < 4; ++i)
	{
		blurHorizontalCS->SetShader(commandList);
		blurHorizontalCS->SetConstants(commandList, &blurRadius, 1, 0);
		blurHorizontalCS->SetConstants(commandList, &focusPlane, 1, 1);
		blurHorizontalCS->SetConstants(commandList, &focalLength, 1, 2);
		blurHorizontalCS->SetConstants(commandList, weights.data(), (UINT)weights.size(), 3);

		blurHorizontalCS->SetTextureSRV(commandList, input);
		blurHorizontalCS->SetTextureSRVOffset(commandList, inputSRV);
		blurHorizontalCS->SetTextureUAV(commandList, outputUAV);
		UINT numGroupsX = (UINT)ceilf(width / 256.f);
		blurHorizontalCS->Dispatch(commandList, numGroupsX, height, 1);

		input = texturePool->GetSRV(0);
		outputUAV = texturePool->GetUAV(1);

		blurVerticalCS->SetShader(commandList);
		blurVerticalCS->SetConstants(commandList, &blurRadius, 1, 0);
		blurVerticalCS->SetConstants(commandList, &focusPlane, 1, 1);
		blurVerticalCS->SetConstants(commandList, &focalLength, 1, 2);
		blurVerticalCS->SetConstants(commandList, weights.data(), (UINT)weights.size(), 3);

		blurVerticalCS->SetTextureSRV(commandList, input);
		blurVerticalCS->SetTextureSRVOffset(commandList, inputSRV);
		blurVerticalCS->SetTextureUAV(commandList, outputUAV);
		UINT numGroupsY = (UINT)ceilf(height / 256.f);
		blurVerticalCS->Dispatch(commandList, width, numGroupsY, 1);
	}

	return texturePool->GetSRV(1);
}
