#pragma once
#include "../stdafx.h"
#include "../DirectXHelper.h"
#include <vector>
#include "Entity.h"
#include "ConstantBuffer.h"
#include "Camera.h"

class ConstantBufferWrapper
{
	ID3D12Resource* constantBuffer;
	int bufferSize;
	char* vAddressBegin;
public:
	ConstantBufferWrapper()
	{
		constantBuffer = nullptr;
	};
	void Initialize(ID3D12Resource* buffer, const int& bSize)
	{
		bufferSize = bSize;
		constantBuffer = buffer;
		CD3DX12_RANGE readRange(0, 0);
		buffer->Map(0, &readRange, reinterpret_cast<void**>(&vAddressBegin));
	}
	void CopyData(void* data, int size, int index)
	{
		char* ptr = reinterpret_cast<char*>(vAddressBegin) + bufferSize * index;
		memcpy(ptr, data, size);
	}

	~ConstantBufferWrapper()
	{
		/*if (constantBuffer)
			constantBuffer->Unmap(0, nullptr);*/
	}
};

class DeferredRenderer
{
	ID3D12Device *device;
	const static int numRTV = 3;

	ID3D12RootSignature* rootSignature;

	ID3D12Resource* gBufferTextures[numRTV];
	ID3D12Resource* depthStencilTexture;

	ID3D12PipelineState* deferredPSO;
	ID3D12PipelineState* dirLightPassPSO;
	ID3D12PipelineState* shapeLightPassPSO;

	CDescriptorHeapWrapper rtvHeap;
	CDescriptorHeapWrapper dsvHeap;
	CDescriptorHeapWrapper srvHeap;
	CDescriptorHeapWrapper gBufferHeap;
	CDescriptorHeapWrapper cbHeap;
	CDescriptorHeapWrapper pixelCbHeap;

	ConstantBufferWrapper cbWrapper;
	ConstantBufferWrapper pixelCbWrapper;

	//Constant buffer must be larger than 256 bytes
	static const int ConstantBufferSize = (sizeof(ConstantBuffer) + 255) & ~255;;
	static const int PixelConstantBufferSize = (sizeof(PixelConstantBuffer) + 255) & ~255;

	ID3D12Resource *lightCB;
	ID3D12Resource *worldViewCB;
	Mesh* sphereMesh;

	DXGI_FORMAT mDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT mRtvFormat[3] = { DXGI_FORMAT_R11G11B10_FLOAT,DXGI_FORMAT_R8G8B8A8_SNORM,DXGI_FORMAT_R8G8B8A8_UNORM };
	float mClearColor[4] = { 0.0,0.0f,0.0f,1.0f };
	float mClearDepth = 1.0f;

	UINT viewportWidth;
	UINT viewportHeight;

	std::vector<Entity*> entities;
	Camera* camera;

	void CreateCB();
	void CreateViews();
	void CreatePSO();
	void CreateLightPassPSO();
	void CreateRTV();
	void CreateDSV();
	void CreateRootSignature();
	void Draw(Mesh* m, const ConstantBuffer& cb, ID3D12GraphicsCommandList* commandList);
public:
	DeferredRenderer(ID3D12Device *dxDevice, int width, int height);
	void SetSRV(ID3D12Resource* textureSRV, DXGI_FORMAT format, int index);
	void Initialize(ID3D12GraphicsCommandList* command);
	void SetGBUfferPSO(ID3D12GraphicsCommandList* command, std::vector<Entity*> entities, Camera* camera, const PixelConstantBuffer& pixelCb);
	void SetLightPassPSO(ID3D12GraphicsCommandList* command, const PixelConstantBuffer& pixelCb);
	void Draw(ID3D12GraphicsCommandList* commandList);
	void DrawLightPass(ID3D12GraphicsCommandList* commandList);
	void UpdateConstantBuffer(const PixelConstantBuffer& pixelBuffer, ID3D12GraphicsCommandList* command);
	void UpdateConstantBufferPerObject(ConstantBuffer& buffer, int index);

	~DeferredRenderer();
};