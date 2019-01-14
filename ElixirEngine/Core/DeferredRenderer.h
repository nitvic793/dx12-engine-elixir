#pragma once
#include "../stdafx.h"
#include "../DirectXHelper.h"
#include <vector>
#include "Entity.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "../Texture.h"
#include "../FrameManager.h"
#include "../SystemResourceManager.h"


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

enum DefaultRootParameterSlotType {
	RootSigCBVertex0 = 0,
	RootSigCBPixel0,
	RootSigSRVPixel1,
	RootSigCBVertex1,
};

typedef GBufferRenderTargetOrder GBufferType;

class DeferredRenderer
{
	ID3D12Device *device;
	const static int numRTV = RTV_ORDER_COUNT;
	int constBufferIndex = 0;
	int shadowMapSize = 4096;
	const static int shadowMapCount = 32;

	ID3D12RootSignature* rootSignature;
	//Keeps track of the SRV count in SRV Heap
	uint32_t srvHeapIndex;

	SystemResourceManager*			sysRM;
	std::unique_ptr<FrameManager>	frame;
	FrameHeapParameters				frameHeapParams;

	ID3D12Resource* gBufferTextures[numRTV];
	ID3D12Resource* depthStencilTexture;
	ID3D12Resource* postProcessTexture;

	ID3D12Resource* selectedDepthTexture;
	ID3D12Resource* selectedOutlineTexture;

	ID3D12PipelineState* deferredPSO;
	ID3D12PipelineState* dirLightPassPSO;
	ID3D12PipelineState* shapeLightPassPSO;
	ID3D12PipelineState* screenQuadPSO;
	ID3D12PipelineState* skyboxPSO;
	ID3D12PipelineState* shadowMapDirLightPSO;
	ID3D12PipelineState* shadowMapPointLightPSO;
	ID3D12PipelineState* selectionFilterPSO;

	CDescriptorHeapWrapper pRTVHeap;

	CDescriptorHeapWrapper gRTVHeap;
	CDescriptorHeapWrapper dsvHeap;
	CDescriptorHeapWrapper srvHeap;
	CDescriptorHeapWrapper gBufferHeap;
	CDescriptorHeapWrapper samplerHeap;
	CDescriptorHeapWrapper cbHeap;
	CDescriptorHeapWrapper pixelCbHeap;

	PerFrameConstantBuffer frameCB;
	//Shadows
	PointShadowBuffer pShadowBuffer;
	CDescriptorHeapWrapper shadowResHeap;
	CDescriptorHeapWrapper shadowRTVHeap;
	CDescriptorHeapWrapper shadowDSVHeap;
	CDescriptorHeapWrapper shadowCbHeap;

	ID3D12Resource* shadowMaps[shadowMapCount];
	ID3D12Resource* shadowPosTextures[shadowMapCount];

	ID3D12Resource* shadowMapTexture;
	ID3D12Resource* shadowPosTexture;
	ID3D12Resource* shadowMapPointTexture;
	ID3D12Resource* shadowPosPointTexture;
	ID3D12Resource* shadowCB;

	ConstantBufferWrapper shadowCBWrapper;

	//CBs
	ConstantBufferWrapper cbWrapper;
	ConstantBufferWrapper pixelCbWrapper;
	ConstantBufferWrapper perFrameCbWrapper;

	std::unique_ptr<Texture> selectedDepthBufferSRV;
	std::unique_ptr<Texture> selectedOutlineSRV;
	std::unique_ptr<Texture> gDepthSRV;
	Texture* resultUAV;
	Texture* resultSRV;
	Texture* postProcessUAV;
	Texture* postProcessSRV;
	
	XMFLOAT4X4 shadowViewTransposed;
	XMFLOAT4X4 shadowProjTransposed;

	std::vector<Texture*> textureVector;
	std::vector<Texture*> gBufferTextureVector;

	static const int ConstBufferCount = 64;
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
	void CreateSkyboxPSO();
	void CreateLightPassPSO();
	void CreateScreenQuadPSO();
	void CreateRTV();
	void CreateDSV();
	void CreateRootSignature();
	void CreateShadowBuffers();
	void CreateSelectionFilterBuffers();
	void Draw(Mesh* m, ID3D12GraphicsCommandList* commandList);
	void PrepareGPUHeap(std::vector<Entity*> entities, PixelConstantBuffer& pixelCb);
public:
	DeferredRenderer(ID3D12Device *dxDevice, int width, int height);

	void ResetRenderTargetStates(ID3D12GraphicsCommandList* command);
	void SetSRV(ID3D12Resource* textureSRV, int index, bool isTextureCube = false);
	void SetIBLTextures(ID3D12Resource* irradianceTextureCube, ID3D12Resource* prefilterTextureCube, ID3D12Resource* brdfLUTTexture);

	uint32_t SetSRV(ID3D12Resource* textureSRV, bool isTextureCube = false);
	uint32_t SetSRV(ID3D12Resource* textureSRV, DXGI_FORMAT format, bool isTextureCube = false);
	uint32_t SetUAV(ID3D12Resource* textureSRV, bool isTextureCube = false, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	uint32_t SetSRVs(ID3D12Resource** textureSRV, int textureCount, bool isTextureCube = false);
	
	uint32_t				GetHeight();
	uint32_t				GetWidth();
	Texture*				GetSelectionOutlineSRV();
	Texture*				GetSelectionDepthBufferSRV();
	Texture*				GetResultUAV();
	Texture*				GetResultSRV();
	Texture*				GetPostProcessSRV();
	Texture*				GetPostProcessUAV();
	Texture*				GetGBufferDepthSRV();
	std::vector<Texture*>	GetTexturesArrayForPost();
	Texture*				GetGBufferTextureSRV(GBufferType gBufferType);
	ID3D12RootSignature*	GetRootSignature();

	void Initialize(ID3D12GraphicsCommandList* command);
	void SetGBUfferPSO(ID3D12GraphicsCommandList* command, Camera* camera, const PixelConstantBuffer& pixelCb);
	void RenderLightPass(ID3D12GraphicsCommandList* command, const PixelConstantBuffer& pixelCb);
	void RenderLightShapePass(ID3D12GraphicsCommandList* command, PixelConstantBuffer& pixelCb);
	void RenderAmbientPass(ID3D12GraphicsCommandList* clist);

	void RenderSelectionDepthBuffer(ID3D12GraphicsCommandList* commandList, std::vector<Entity*> entities, Camera* camera);
	void RenderShadowMap(ID3D12GraphicsCommandList* commandList, std::vector<Entity*> entities);
	void Draw(ID3D12GraphicsCommandList* commandList, std::vector<Entity*> entities);
	void DrawSkybox(ID3D12GraphicsCommandList* commandList, Texture* skybox);
	void DrawScreenQuad(ID3D12GraphicsCommandList* commandList);
	void DrawLightShapePass(ID3D12GraphicsCommandList* commandList, PixelConstantBuffer & pixelCb);
	void DrawResult(ID3D12GraphicsCommandList* commandList, D3D12_CPU_DESCRIPTOR_HANDLE &rtvHandle);
	void DrawResult(ID3D12GraphicsCommandList* commandList, D3D12_CPU_DESCRIPTOR_HANDLE &rtvHandle, Texture* resultTex);

	void StartFrame(ID3D12GraphicsCommandList* commandList);
	void PrepareFrame(std::vector<Entity*> entities, Camera* camera, PixelConstantBuffer& pixelCb);
	void TransitionToPostProcess(ID3D12GraphicsCommandList* commandList);
	void EndFrame(ID3D12GraphicsCommandList* commandList);

	FrameHeapParameters		GetFrameHeapParameters();
	FrameManager*			GetFrameManager();

	CDescriptorHeapWrapper& GetSRVHeap();
	CDescriptorHeapWrapper& GetGBufferHeap();
	CDescriptorHeapWrapper& GetCBHeap();
	~DeferredRenderer();
};