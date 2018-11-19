#pragma once

#include "stdafx.h"
#include "ComputeProcess.h"
#include "TexturePool.h"
#include "DirectXHelper.h"
#include "Core/Camera.h"
class DeferredRenderer;

class SunRaysPass
{
	std::unique_ptr<ComputeProcess> occlusionPassCS;
	std::unique_ptr<ComputeProcess> sunRaysPassCS;
	std::unique_ptr<ComputeProcess> compositeRaysCS;

	ID3D12PipelineState*	sunRaysPSO;
	ID3D12Resource*			constantBuffer;
	ComputeCore*			computeCore;
	DeferredRenderer*		renderer;
	ConstantBufferWrapper	cbWrapper;
	CDescriptorHeapWrapper	cbHeap;
	void CreatePSO();
	void CreateCB();
public:
	SunRaysPass(ComputeCore* core, DeferredRenderer* renderContext);
	Texture* GetOcclusionTexture(ID3D12GraphicsCommandList* commandList, Texture* depthSRV, TexturePool *texturePool);
	Texture* Apply(ID3D12GraphicsCommandList* commandList, Texture* depthSRV, Texture* pixels, TexturePool* texturePool, Camera* camera);
	~SunRaysPass();
};

