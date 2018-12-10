#pragma once
#include "stdafx.h"
#include "ComputeProcess.h"
#include "TexturePool.h"

class CompositeTextures
{
	ComputeCore*					core;
	std::unique_ptr<ComputeProcess>	compositeCS;
public:
	CompositeTextures(ComputeCore* computeCore);
	Texture* Composite(ID3D12GraphicsCommandList* command, Texture* inputA, Texture* inputB, TexturePool* pool);
	~CompositeTextures();
};

