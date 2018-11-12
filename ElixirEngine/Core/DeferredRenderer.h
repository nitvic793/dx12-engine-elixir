#pragma once
#include "../stdafx.h"
#include "../DirectXHelper.h"
#include <vector>
#include "Entity.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "../Texture.h"

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

enum GBufferRenderTargetOrder
{
	RTV_ORDER_ALBEDO = 0,
	RTV_ORDER_NORMAL,
	RTV_ORDER_WORLDPOS,
	RTV_ORDER_ROUGHNESS,
	RTV_ORDER_METALNESS,
	RTV_ORDER_LIGHTSHAPE,
	RTV_ORDER_QUAD,
	RTV_ORDER_COUNT
};

typedef GBufferRenderTargetOrder GBufferType;

class DeferredRenderer
{
	ID3D12Device *device;
	const static int numRTV = 7;
	int constBufferIndex = 0;
	ID3D12RootSignature* rootSignature;
	uint32_t srvHeapIndex;

	ID3D12Resource* gBufferTextures[numRTV];
	ID3D12Resource* depthStencilTexture;
	ID3D12Resource* prefilterTexture;
	ID3D12Resource* postProcessTexture;

	ID3D12PipelineState* deferredPSO;
	ID3D12PipelineState* dirLightPassPSO;
	ID3D12PipelineState* shapeLightPassPSO;
	ID3D12PipelineState* screenQuadPSO;
	ID3D12PipelineState* skyboxPSO;
	ID3D12PipelineState* prefilterEnvMapPSO;

	CDescriptorHeapWrapper prefilterRTVHeap;
	CDescriptorHeapWrapper rtvHeap;
	CDescriptorHeapWrapper dsvHeap;
	CDescriptorHeapWrapper srvHeap;
	CDescriptorHeapWrapper gBufferHeap;
	CDescriptorHeapWrapper cbHeap;
	CDescriptorHeapWrapper pixelCbHeap;

	ConstantBufferWrapper cbWrapper;
	ConstantBufferWrapper pixelCbWrapper;
	ConstantBufferWrapper perFrameCbWrapper;

	Texture* resultUAV;
	Texture* resultSRV;
	Texture* postProcessUAV;
	Texture* postProcessSRV;

	std::vector<Texture*> textureVector;
	std::vector<Texture*> gBufferTextureVector;

	static const int ConstBufferCount = 32;
	//Constant buffer must be larger than 256 bytes
	static const int ConstantBufferSize = (sizeof(ConstantBuffer) + 255) & ~255;
	static const int PixelConstantBufferSize = (sizeof(PixelConstantBuffer) + 255) & ~255;

	ID3D12Resource *lightCB;
	ID3D12Resource *worldViewCB;
	ID3D12Resource *perFrameCB;
	Mesh* sphereMesh;
	Mesh* cubeMesh;

	DXGI_FORMAT mDsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT mRtvFormat[numRTV] = {
		DXGI_FORMAT_R32G32B32A32_FLOAT, //Albedo
		DXGI_FORMAT_R8G8B8A8_SNORM, //Normal
		DXGI_FORMAT_R32G32B32A32_FLOAT, //WorldPos
		DXGI_FORMAT_R8_UNORM, //Roughness
		DXGI_FORMAT_R8_UNORM, //Metalness
		DXGI_FORMAT_R32G32B32A32_FLOAT, //LightShape
		DXGI_FORMAT_R32G32B32A32_FLOAT //Result
	};
	float mClearColor[4] = { 0.0,0.0f,0.0f,1.0f };
	float mClearDepth = 1.0f;

	UINT viewportWidth;
	UINT viewportHeight;

	std::vector<Entity*> entities;
	Camera* camera;

	void CreateCB();
	void CreateViews();
	void CreatePSO();
	void CreatePrefilterEnvironmentPSO();
	void CreatePrefilterResources(ID3D12GraphicsCommandList* command);
	void CreateSkyboxPSO();
	void CreateLightPassPSO();
	void CreateScreenQuadPSO();
	void CreateRTV();
	void CreateDSV();
	void CreateRootSignature();
	void Draw(Mesh* m, const ConstantBuffer& cb, ID3D12GraphicsCommandList* commandList);
public:
	DeferredRenderer(ID3D12Device *dxDevice, int width, int height);
	void ResetRenderTargetStates(ID3D12GraphicsCommandList* command);

	void SetSRV(ID3D12Resource* textureSRV, int index, bool isTextureCube = false);
	uint32_t SetSRV(ID3D12Resource* textureSRV, bool isTextureCube = false);
	uint32_t SetUAV(ID3D12Resource* textureSRV, bool isTextureCube = false);
	uint32_t SetSRVs(ID3D12Resource** textureSRV, int textureCount, bool isTextureCube = false);
	void SetIBLTextures(ID3D12Resource* irradianceTextureCube, ID3D12Resource* prefilterTextureCube, ID3D12Resource* brdfLUTTexture);
	Texture*				GetResultUAV();
	Texture*				GetResultSRV();
	Texture*				GetPostProcessSRV();
	Texture*				GetPostProcessUAV();
	std::vector<Texture*>	GetTexturesArrayForPost();
	Texture*				GetGBufferTextureSRV(GBufferType gBufferType);

	void Initialize(ID3D12GraphicsCommandList* command);
	void GeneratePreFilterEnvironmentMap(ID3D12GraphicsCommandList* command, int envTextureIndex);
	void SetGBUfferPSO(ID3D12GraphicsCommandList* command, Camera* camera, const PixelConstantBuffer& pixelCb);
	void SetLightPassPSO(ID3D12GraphicsCommandList* command, const PixelConstantBuffer& pixelCb);
	void SetLightShapePassPSO(ID3D12GraphicsCommandList* command, const PixelConstantBuffer& pixelCb);

	void Draw(ID3D12GraphicsCommandList* commandList, std::vector<Entity*> entities);
	void DrawSkybox(ID3D12GraphicsCommandList* commandList, Texture* skybox);
	void DrawLightPass(ID3D12GraphicsCommandList* commandList);
	void DrawLightShapePass(ID3D12GraphicsCommandList* commandList, const PixelConstantBuffer & pixelCb);
	void DrawResult(ID3D12GraphicsCommandList* commandList, D3D12_CPU_DESCRIPTOR_HANDLE &rtvHandle);
	void DrawResult(ID3D12GraphicsCommandList* commandList, D3D12_CPU_DESCRIPTOR_HANDLE &rtvHandle, Texture* resultTex);

	void UpdateConstantBuffer(const PixelConstantBuffer& pixelBuffer, ID3D12GraphicsCommandList* command);
	void UpdateConstantBufferPerObject(ConstantBuffer& buffer, int index);
	CDescriptorHeapWrapper& GetSRVHeap();
	CDescriptorHeapWrapper& GetGBufferHeap();
	~DeferredRenderer();
};