#include "stdafx.h"
#include "SunRaysPass.h"
#include "Core/DeferredRenderer.h"

void SunRaysPass::CreatePSO()
{
	auto device = computeCore->GetDevice();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"ScreenQuadVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"SunRaysPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = nullptr;
	descPipelineState.InputLayout.NumElements = 0;// _countof(inputLayout);
	descPipelineState.pRootSignature = renderer->GetRootSignature();
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.DepthStencilState.DepthEnable = false;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState.DepthClipEnable = false;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descPipelineState.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&sunRaysPSO));
}

SunRaysPass::SunRaysPass(ComputeCore* core, DeferredRenderer* renderContext)
{
	computeCore = core;
	renderer = renderContext;
	occlusionPassCS = std::make_unique<ComputeProcess>(core, L"OcclusionPassCS.cso");
	sunRaysPassCS = std::make_unique<ComputeProcess>(core, L"SunRaysCS.cso");
	CreatePSO();
}

Texture* SunRaysPass::GetOcclusionTexture(ID3D12GraphicsCommandList* commandList, Texture * depthSRV, TexturePool *texturePool)
{
	auto uavTex = texturePool->GetUAV(3);
	auto srvTex = texturePool->GetSRV(3);
	occlusionPassCS->SetShader(commandList);
	occlusionPassCS->SetTextureSRV(commandList, depthSRV);
	occlusionPassCS->SetTextureUAV(commandList, uavTex);
	occlusionPassCS->Dispatch(commandList, 1280, 720, 1);
	return srvTex;
}

Texture* SunRaysPass::Apply(ID3D12GraphicsCommandList* commandList, Texture* depthSRV, TexturePool* texturePool)
{	//TODO : SunRaysPSO, RTV in TexturePool?
	auto outUAV = texturePool->GetUAV(4);
	auto outSRV = texturePool->GetSRV(4);
	auto outRTV = texturePool->GetRTVHandle(4);
	float mClearColor[4] = { 0.0,0.0f,0.0f,1.0f };
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(outSRV->GetTextureResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET ));
	auto occlusionTex = GetOcclusionTexture(commandList, depthSRV, texturePool);
	commandList->SetPipelineState(sunRaysPSO);
	ID3D12DescriptorHeap* heaps[] = { occlusionTex->GetTextureDescriptorHeap()->pDescriptorHeap.Get() };
	commandList->ClearRenderTargetView(outRTV, mClearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &outRTV, true, nullptr);
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(2, occlusionTex->GetGPUDescriptorHandle());
	renderer->DrawLightPass(commandList);
	//sunRaysPassCS->SetShader(commandList);
	//sunRaysPassCS->SetTextureSRV(commandList, occlusionTex);
	//sunRaysPassCS->SetTextureUAV(commandList, outUAV);
	//sunRaysPassCS->Dispatch(commandList, 1280, 720, 1);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(outSRV->GetTextureResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	return outSRV;
}


SunRaysPass::~SunRaysPass()
{
	sunRaysPSO->Release();
}
