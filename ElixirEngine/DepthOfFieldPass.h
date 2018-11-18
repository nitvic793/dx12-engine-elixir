#pragma once
#include "stdafx.h"
#include "ComputeProcess.h"
#include "TexturePool.h"

class DepthOfFieldPass
{
	ComputeProcess* dofCS;
	ComputeCore*	computeCore;
public:
	DepthOfFieldPass(ComputeCore* core);
	Texture* Apply(ID3D12GraphicsCommandList* commandList, Texture* sharpSRV, Texture* blurSRV, TexturePool* texturePool, float focusPlaneZ, float scale);
	~DepthOfFieldPass();
};

