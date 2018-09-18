#pragma once
#include "../stdafx.h"
#include "../DirectXHelper.h"

#include "ConstantBuffer.h"

class DeferredRenderer
{
	ID3D12Device *device;
	const static int numRTV = 3;

	ID3D12RootSignature* rootSignature;

	ID3D12Resource* gBufferTextures[numRTV];
	ID3D12Resource* depthStencilTexture;

	ID3D12PipelineState* deferredPSO;
	ID3D12PipelineState* lightPassPSO;

	CDescriptorHeapWrapper rtvHeap;
	CDescriptorHeapWrapper dsvHeap;
	CDescriptorHeapWrapper srvHeap;
	CDescriptorHeapWrapper cbvSrvHeap;

	ID3D12Resource *lightCB;
	ID3D12Resource *worldViewCB;

	DXGI_FORMAT mDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT mRtvFormat[3] = { DXGI_FORMAT_R11G11B10_FLOAT,DXGI_FORMAT_R8G8B8A8_SNORM,DXGI_FORMAT_R8G8B8A8_UNORM };
	float mClearColor[4] = { 0.0,0.0f,0.0f,1.0f };
	float mClearDepth = 1.0f;

	UINT viewportWidth;
	UINT viewportHeight;

	void CreateCB();
	void CreateViews();
	void CreatePSO();
	void CreateLightPassPSO();
	void CreateRTV();
	void CreateDSV();
	void CreateRootSignature();

public:
	DeferredRenderer(ID3D12Device *dxDevice, int width, int height);
	void SetSRV(ID3D12Resource* textureSRV, DXGI_FORMAT format);
	void Initialize();
	void SetGBUfferPSO(ID3D12GraphicsCommandList* command);
	void UpdateConstantBuffer(ConstantBuffer& buffer, PixelConstantBuffer& pixelBuffer, ID3D12GraphicsCommandList* command);
	void UpdateConstantBufferPerObject(ConstantBuffer& buffer, int index);
	~DeferredRenderer();
};