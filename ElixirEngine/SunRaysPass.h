#pragma once

#include "stdafx.h"
#include "ComputeProcess.h"
#include "TexturePool.h"

class DeferredRenderer;

class SunRaysPass
{
	std::unique_ptr<ComputeProcess> occlusionPassCS;
	std::unique_ptr<ComputeProcess> sunRaysPassCS;
	ID3D12PipelineState* sunRaysPSO;
	ComputeCore* computeCore;
	DeferredRenderer* renderer;
	void CreatePSO();
public:
	SunRaysPass(ComputeCore* core, DeferredRenderer* renderContext);
	Texture* GetOcclusionTexture(ID3D12GraphicsCommandList* commandList, Texture* depthSRV, TexturePool *texturePool);
	Texture* Apply(ID3D12GraphicsCommandList* commandList, Texture* depthSRV, TexturePool* texturePool);
	~SunRaysPass();
};

