#pragma once
#include "ComputeProcess.h"
#include "TexturePool.h"

class EdgeFilter
{
	std::unique_ptr<ComputeProcess> edgeDetectionCS;
	ComputeCore* computeCore;
public:
	EdgeFilter(ComputeCore* core);
	Texture* Apply(ID3D12GraphicsCommandList* command, Texture* depthBuffer, Texture* pixels, TexturePool* pool);
	~EdgeFilter();
};

