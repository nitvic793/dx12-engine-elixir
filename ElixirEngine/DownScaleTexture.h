#pragma once

#include "stdafx.h"
#include "ComputeProcess.h"
#include "TexturePool.h"

class DownScaleTexture
{
	ComputeCore*					computeCore;
	std::unique_ptr<ComputeProcess> downScaleCS;
public:
	DownScaleTexture(ComputeCore* core);
	Texture* Apply(ID3D12GraphicsCommandList* clist, Texture* input, TexturePool* pool);
	~DownScaleTexture();
};

