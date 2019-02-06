#include "DeferredRenderer.h"
#include "ShaderManager.h"
#include "Vertex.h"
#include "DirectXTex.h"
#include "../InputLayout.h"

struct PrefilterPixelConstBuffer
{
	float roughness;
};

DeferredRenderer::DeferredRenderer(ID3D12Device* dxDevice, int width, int height) :
	device(dxDevice),
	viewportHeight(height),
	viewportWidth(width),
	constBufferIndex(0),
	srvHeapIndex(0)
{
}

void DeferredRenderer::ResetRenderTargetStates(ID3D12GraphicsCommandList* command)
{
	for (int i = 0; i < numRTV; i++)
		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[i], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowPosTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	constBufferIndex = 0;
}

void DeferredRenderer::SetSRV(ID3D12Resource* textureSRV, int index, bool isTextureCube)
{
	CreateShaderResourceView(device, textureSRV, srvHeap.handleCPU(index), isTextureCube);
}

uint32_t DeferredRenderer::SetSRV(ID3D12Resource* textureSRV, bool isTextureCube)
{
	auto index = srvHeapIndex;
	CreateShaderResourceView(device, textureSRV, srvHeap.handleCPU(index), isTextureCube);
	srvHeapIndex++;
	return index;
}

uint32_t DeferredRenderer::SetSRV(ID3D12Resource * textureSRV, DXGI_FORMAT format, bool isTextureCube)
{
	auto viewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	if (isTextureCube) viewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Texture2D.MipLevels = 1;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Format = format;
	descSRV.ViewDimension = viewDimension;
	descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	auto index = srvHeapIndex;
	device->CreateShaderResourceView(textureSRV, &descSRV, srvHeap.handleCPU(index));
	srvHeapIndex++;
	return index;
}

uint32_t DeferredRenderer::SetUAV(ID3D12Resource * textureSRV, bool isTextureCube, DXGI_FORMAT format)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	UAVDesc.Format = format;
	UAVDesc.Buffer.NumElements = 1;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	auto index = srvHeapIndex;
	device->CreateUnorderedAccessView(textureSRV, nullptr, &UAVDesc, srvHeap.handleCPU(index));
	srvHeapIndex++;
	return index;
}

uint32_t DeferredRenderer::SetSRVs(ID3D12Resource** textureSRV, int textureCount, bool isTextureCube)
{
	auto index = srvHeapIndex;
	auto heapIndexInc = srvHeapIndex;
	for (uint32_t i = index; i < index + textureCount; ++i)
	{
		CreateShaderResourceView(device, textureSRV[i], srvHeap.handleCPU(i), isTextureCube);
		heapIndexInc++;
	}
	srvHeapIndex = heapIndexInc;
	return index;
}

uint32_t DeferredRenderer::GetHeight()
{
	return viewportHeight;
}

uint32_t DeferredRenderer::GetWidth()
{
	return viewportWidth;
}

void DeferredRenderer::SetIBLTextures(ID3D12Resource* irradianceTextureCube, ID3D12Resource* prefilterTextureCube, ID3D12Resource* brdfLUTTexture)
{
	// numRTV is reserved for Depth Target
	int irradianceIndex = numRTV + 1;
	int brdfTextureIndex = numRTV + 2;
	int prefilterIndex = numRTV + 3;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(irradianceTextureCube, &srvDesc, gBufferHeap.handleCPU(irradianceIndex));


	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	device->CreateShaderResourceView(brdfLUTTexture, &srvDesc, gBufferHeap.handleCPU(brdfTextureIndex));

	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MipLevels = 10;
	device->CreateShaderResourceView(prefilterTextureCube, &srvDesc, gBufferHeap.handleCPU(prefilterIndex));
}

Texture * DeferredRenderer::GetSelectionOutlineSRV()
{
	return selectedOutlineSRV.get();
}

Texture * DeferredRenderer::GetSelectionDepthBufferSRV()
{
	return selectedDepthBufferSRV.get();
}

Texture * DeferredRenderer::GetResultUAV()
{
	return resultUAV;
}

Texture * DeferredRenderer::GetResultSRV()
{
	return resultSRV;
}

Texture * DeferredRenderer::GetPostProcessSRV()
{
	return postProcessSRV;
}

Texture * DeferredRenderer::GetPostProcessUAV()
{
	return postProcessUAV;
}

Texture * DeferredRenderer::GetGBufferDepthSRV()
{
	return gDepthSRV.get();
}

std::vector<Texture*> DeferredRenderer::GetTexturesArrayForPost()
{
	return textureVector;
}

Texture* DeferredRenderer::GetGBufferTextureSRV(GBufferType gBufferType)
{
	return gBufferTextureVector[gBufferType];
}

ID3D12RootSignature * DeferredRenderer::GetRootSignature()
{
	return rootSignature;
}

void DeferredRenderer::Initialize(ID3D12GraphicsCommandList* command)
{
	sysRM = SystemResourceManager::GetInstance();
	resourceManager = ResourceManager::GetInstance();
	CreateCB();
	CreateViews();
	CreateRootSignature();

	CreatePSO();
	CreateSkyboxPSO();
	CreateLightPassPSO();
	CreateScreenQuadPSO();

	CreateRTV();
	CreateDSV();
	CreateShadowBuffers();
	CreateSelectionFilterBuffers();

	frame = std::unique_ptr<FrameManager>(new FrameManager(device));
	sphereMesh = new Mesh("../../Assets/sphere.obj", device, command);
	cubeMesh = new Mesh("../../Assets/cube.obj", device, command);
}

void DeferredRenderer::SetGBUfferPSO(ID3D12GraphicsCommandList* command, Camera* camera, const PixelConstantBuffer& pixelCb)
{
	this->camera = camera;
	this->entities = entities;

	command->SetPipelineState(deferredPSO);
	for (int i = 0; i < numRTV; i++)
		command->ClearRenderTargetView(gRTVHeap.handleCPU(i), mClearColor, 0, nullptr);

	command->ClearDepthStencilView(dsvHeap.hCPUHeapStart, D3D12_CLEAR_FLAG_DEPTH, mClearDepth, 0xff, 0, nullptr);
	command->OMSetRenderTargets(numRTV + 1, &gRTVHeap.hCPUHeapStart, true, &dsvHeap.hCPUHeapStart);
	command->SetGraphicsRootSignature(rootSignature);

	pixelCbWrapper.CopyData((void*)&pixelCb, sizeof(PixelConstantBuffer), 0);
}

void DeferredRenderer::RenderLightPass(ID3D12GraphicsCommandList * command, const PixelConstantBuffer & pixelCb)
{
	/*for (int i = 0; i < numRTV; i++)
		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));*/
		//command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_LIGHTSHAPE], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	command->SetPipelineState(dirLightPassPSO);

	//command->ClearRenderTargetView(rtvHeap.handleCPU(RTV_ORDER_QUAD), mClearColor, 0, nullptr);
	command->OMSetRenderTargets(1, &gRTVHeap.handleCPU(RTV_ORDER_LIGHTSHAPE), true, nullptr);

	ID3D12DescriptorHeap* samplerHeaps[] = { samplerHeap.pDescriptorHeap.Get() };
	ID3D12DescriptorHeap* ppHeaps[] = { frame->GetDescriptorHeap() };

	command->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.GBuffer)); // Set G-Buffer Textures
	command->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frame->GetGPUHandle(frameHeapParams.PixelCB));// pixelCbHeap.handleGPU(0)); //Set Pixel Shader Constants

	//command->SetDescriptorHeaps(1, samplerHeaps);
	//command->SetGraphicsRootDescriptorTable(4, samplerHeap.hGPUHeapStart); //Set Shadow Sampler
	DrawScreenQuad(command);
	command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_LIGHTSHAPE], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DeferredRenderer::RenderLightShapePass(ID3D12GraphicsCommandList * command, PixelConstantBuffer & pixelCb)
{
	for (int i = 0; i < numRTV; i++)
		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));

	// (numRTV - 2)th texture and RTV is being used to store the light shape pass
	command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_LIGHTSHAPE], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	command->ClearRenderTargetView(gRTVHeap.handleCPU(RTV_ORDER_LIGHTSHAPE), mClearColor, 0, nullptr);
	command->OMSetRenderTargets(1, &gRTVHeap.handleCPU(RTV_ORDER_LIGHTSHAPE), true, nullptr);
	command->SetPipelineState(shapeLightPassPSO);
	command->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frame->GetGPUHandle(frameHeapParams.PixelCB));
	command->SetGraphicsRootDescriptorTable(RootSigCBVertex1, frame->GetGPUHandle(frameHeapParams.PerFrameCB));
	command->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.GBuffer));

	DrawLightShapePass(command, pixelCb);
}

void DeferredRenderer::RenderAmbientPass(ID3D12GraphicsCommandList * command)
{
	command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_QUAD], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	command->SetPipelineState(sysRM->GetPSO(StringID("ambientPass")));

	//command->ClearRenderTargetView(rtvHeap.handleCPU(RTV_ORDER_QUAD), mClearColor, 0, nullptr);
	command->OMSetRenderTargets(1, &gRTVHeap.handleCPU(RTV_ORDER_QUAD), true, nullptr);
	command->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.GBuffer)); // Set G-Buffer Textures
	command->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frame->GetGPUHandle(frameHeapParams.PixelCB));// pixelCbHeap.handleGPU(0)); //Set Pixel Shader Constants
	DrawScreenQuad(command);
}

void DeferredRenderer::RenderSelectionDepthBuffer(ID3D12GraphicsCommandList* commandList, std::vector<Entity*> entities, Camera* camera)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(selectedDepthTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	commandList->ClearRenderTargetView(pRTVHeap.hCPUHeapStart, mClearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHeap.handleCPU(1), D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, mClearDepth, 0xff, 0, nullptr);
	commandList->OMSetRenderTargets(1, &pRTVHeap.hCPUHeapStart, false, &dsvHeap.handleCPU(1));
	commandList->SetPipelineState(selectionFilterPSO);

	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex1, frame->GetGPUHandle(frameHeapParams.PerFrameCB)); //Per frame const buffer
	for (auto e : entities)
	{
		commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.Entities, e->GetID()));
		Draw(e->GetMesh(), commandList);
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(selectedDepthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DeferredRenderer::RenderShadowMap(ID3D12GraphicsCommandList * commandList, std::vector<Entity*> entities, std::vector<MeshInstanceGroupEntity*> instancedEntities)
{
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = shadowMapSize;
	scissorRect.bottom = shadowMapSize;

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMapTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->ClearDepthStencilView(shadowDSVHeap.handleCPU(0), D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, mClearDepth, 0xff, 0, nullptr);
	commandList->OMSetRenderTargets(0, nullptr, false, &shadowDSVHeap.handleCPU(0));
	commandList->SetPipelineState(shadowMapDirLightPSO);

	int sIndex = 0;
	for (auto e : entities)
	{
		if (!e->CastsShadow()) continue;
		commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.ShadowCB, sIndex));
		Draw(e->GetMesh(), commandList);
		sIndex++;
	}

	commandList->SetPipelineState(sysRM->GetPSO(StringID("shadowInstancedDirLightPSO")));
	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.ShadowCB, 0)); //Set 0th Shadow CB as we only need view and projection from it
	for (auto e : instancedEntities)
	{
		if (!e->CastsShadow()) continue;
		auto meshes = e->GetMeshIDs();
		for (auto meshID : meshes)
		{
			auto mesh = resourceManager->GetMesh(meshID);
			DrawInstanced(e, mesh, commandList);
		}
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMapTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMapPointTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	viewport.Width = (float)1024;
	viewport.Height = (float)1024;
	scissorRect.right = 1024;
	scissorRect.bottom = 1024;
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->ClearDepthStencilView(shadowDSVHeap.handleCPU(1), D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, mClearDepth, 0xff, 0, nullptr);
	commandList->OMSetRenderTargets(0, nullptr, false, &shadowDSVHeap.handleCPU(1));
	commandList->SetPipelineState(shadowMapPointLightPSO);
	sIndex = 0;
	auto shadowFrameIndex = frame->CopyAllocate(1, pointShadowCbHeap, 0);
	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex1, frame->GetGPUHandle(shadowFrameIndex));
	for (auto e : entities)
	{
		if (!e->CastsShadow()) continue;
		commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.ShadowCB, sIndex));
		Draw(e->GetMesh(), commandList);
		sIndex++;
	}
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMapPointTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void DeferredRenderer::Draw(ID3D12GraphicsCommandList* commandList, std::vector<Entity*> entities)
{
	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex1, frame->GetGPUHandle(frameHeapParams.PerFrameCB));
	for (auto e : entities)
	{
		commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.Textures, e->GetMaterial()->GetStartIndex())); //Set start of material texture in root descriptor
		commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.Entities, e->GetID()));
		Draw(e->GetMesh(), commandList);
	}
}

void DeferredRenderer::DrawInstanced(ID3D12GraphicsCommandList * commandList, std::vector<MeshInstanceGroupEntity*> entities)
{
	commandList->SetPipelineState(sysRM->GetPSO(StringID("instancedDeferredPSO")));
	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex1, frame->GetGPUHandle(frameHeapParams.PerFrameCB));
	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.Entities)); //Set the first CB as the world matrix is not used, only view and projection
	for (auto e : entities)
	{
		auto meshes = e->GetMeshIDs();
		auto materials = e->GetMaterialIDs();
		for (size_t i = 0; i < meshes.size(); ++i)
		{
			auto material = resourceManager->GetMaterial(materials[i]);
			commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.Textures, material->GetStartIndex()));
			auto mesh = resourceManager->GetMesh(meshes[i]);
			DrawInstanced(e, mesh, commandList);
		}
	}
}

void DeferredRenderer::DrawSkybox(ID3D12GraphicsCommandList * commandList, Texture* skybox)
{
	commandList->SetPipelineState(skyboxPSO);
	commandList->OMSetRenderTargets(1, &gRTVHeap.handleCPU(RTV_ORDER_QUAD), true, &dsvHeap.hCPUHeapStart);
	commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.Textures, skybox->GetHeapIndex())); //Set skybox texture
	commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.SkyCB)); // set constant buffer with view and projection matrices
	Draw(cubeMesh, commandList);
}

void DeferredRenderer::DrawScreenQuad(ID3D12GraphicsCommandList * commandList)
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.Format = DXGI_FORMAT_R32_UINT;
	ibv.BufferLocation = 0;
	ibv.SizeInBytes = 0;

	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = 0;
	vbv.SizeInBytes = 0;
	vbv.StrideInBytes = 0;
	commandList->IASetVertexBuffers(0, 0, &vbv);
	commandList->IASetIndexBuffer(&ibv);
	commandList->DrawInstanced(4, 1, 0, 0);
}

void DeferredRenderer::DrawLightShapePass(ID3D12GraphicsCommandList * commandList, PixelConstantBuffer & pixelCb)
{
	for (auto i = 0u; i < pixelCb.pointLightCount; ++i)
	{
		commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frame->GetGPUHandle(frameHeapParams.LightShapes, i));
		commandList->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frame->GetGPUHandle(frameHeapParams.PixelCB, i + 1));
		Draw(sphereMesh, commandList);
	}

	//numRTV - 2 is the lightshape pass RTV
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_LIGHTSHAPE], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowPosTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DeferredRenderer::DrawResult(ID3D12GraphicsCommandList* commandList, D3D12_CPU_DESCRIPTOR_HANDLE & rtvHandle)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_QUAD], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	commandList->ClearRenderTargetView(rtvHandle, mClearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	commandList->SetPipelineState(screenQuadPSO);
	commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.GBuffer, RTV_ORDER_QUAD));
	DrawScreenQuad(commandList); // Draws full screen quad with null vertex buffer.
}

void DeferredRenderer::DrawResult(ID3D12GraphicsCommandList * commandList, D3D12_CPU_DESCRIPTOR_HANDLE & rtvHandle, Texture* resultTex)
{
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_QUAD], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	commandList->ClearRenderTargetView(rtvHandle, mClearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	commandList->SetPipelineState(screenQuadPSO);
	commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frame->GetGPUHandle(frameHeapParams.Textures, resultTex->GetHeapIndex()));
	DrawScreenQuad(commandList); // Draws full screen quad with null vertex buffer.
}

void DeferredRenderer::StartFrame(ID3D12GraphicsCommandList* commandList)
{
	frame->StartFrame();
	ID3D12DescriptorHeap* frameHeap[] = { frame->GetDescriptorHeap() };
	commandList->SetDescriptorHeaps(1, frameHeap);
}

void DeferredRenderer::PrepareFrame(std::vector<Entity*> entities, Camera * camera, PixelConstantBuffer & pixelCb)
{
	this->camera = camera;
	PrepareGPUHeap(entities, pixelCb);
}

void DeferredRenderer::TransitionToPostProcess(ID3D12GraphicsCommandList * commandList)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_QUAD], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DeferredRenderer::EndFrame(ID3D12GraphicsCommandList* commandList)
{
	ResetRenderTargetStates(commandList);
	frame->EndFrame();
}

FrameHeapParameters DeferredRenderer::GetFrameHeapParameters()
{
	return frameHeapParams;
}

FrameManager* DeferredRenderer::GetFrameManager()
{
	return frame.get();
}

void DeferredRenderer::Draw(Mesh * m, ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m->GetVertexBufferView());
	commandList->IASetIndexBuffer(&m->GetIndexBufferView());
	commandList->DrawIndexedInstanced(m->GetIndexCount(), 1, 0, 0, 0);
}

void DeferredRenderer::DrawInstanced(MeshInstanceGroupEntity * instanced, Mesh * mesh, ID3D12GraphicsCommandList * commandList)
{
	D3D12_VERTEX_BUFFER_VIEW views[] = { mesh->GetVertexBufferView() , instanced->GetInstanceBufferView() };
	commandList->IASetVertexBuffers(0, 2, views);
	commandList->IASetIndexBuffer(&mesh->GetIndexBufferView());
	commandList->DrawIndexedInstanced(mesh->GetIndexCount(), instanced->GetInstanceCount(), 0, 0, 0);
}

void DeferredRenderer::PrepareGPUHeap(std::vector<Entity*> entities, PixelConstantBuffer & pixelCb)
{
	auto currentGBufferIndex = frame->CopyAllocate(16, gBufferHeap);
	auto srvGpuHeapIndex = frame->CopyAllocate(srvHeapIndex, srvHeap); //Copy textures
	auto index = constBufferIndex;

	//Create Entity Constant Buffers and copy to CBVs
	for (auto e : entities)
	{
		e->SetID(index);
		auto cb = ConstantBuffer
		{
			e->GetWorldViewProjectionTransposed(camera->GetProjectionMatrix(), camera->GetViewMatrix()),
			e->GetWorldMatrixTransposed(),
			camera->GetViewMatrixTransposed(),
			camera->GetProjectionMatrixTransposed(),
			shadowViewTransposed,
			shadowProjTransposed
		};

		cbWrapper.CopyData(&cb, ConstantBufferSize, index);
		index++;
	}

	auto entitiesHeapIndex = frame->CopyAllocate((UINT)entities.size(), cbHeap, constBufferIndex);
	constBufferIndex = index;

	auto dir = -XMVector3Normalize(XMLoadFloat3(&pixelCb.light[0].Direction));
	auto eyePosV = XMVectorSet(0, 0, 0, 0) + 10 * dir;
	XMFLOAT3 eyePos;
	XMStoreFloat3(&eyePos, eyePosV);

	//Create Shadow CBVs
	XMMATRIX shView = XMMatrixLookAtLH(
		eyePosV,	// Start back and in the air
		XMVectorSet(0, 0, 0, 0),	// Look at the origin
		XMVectorSet(0, 1, 0, 0));	// Up is up
	XMStoreFloat4x4(&shadowViewTransposed, XMMatrixTranspose(shView));


	XMMATRIX shProj = XMMatrixOrthographicLH(40.0f, 40.0f, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowProjTransposed, XMMatrixTranspose(shProj));



	//Create Point Light CBVs and corresponding mesh CBVs
	Entity e;
	for (auto i = 0u; i < pixelCb.pointLightCount; ++i)
	{
		e.SetMesh(sphereMesh);
		e.SetPosition(pixelCb.pointLight[i].Position);
		float range = pixelCb.pointLight[i].Range;
		pixelCb.pointLightIndex = i;
		e.SetScale(XMFLOAT3(range, range, range));

		auto cb = ConstantBuffer{ e.GetWorldViewProjectionTransposed(camera->GetProjectionMatrix(), camera->GetViewMatrix()), e.GetWorldMatrixTransposed() };
		pixelCbWrapper.CopyData((void*)(&pixelCb), PixelConstantBufferSize, i + 1);
		cbWrapper.CopyData(&cb, ConstantBufferSize, index);
		index++;
	}

	auto pixelCbHeapIndex = frame->CopyAllocate(pixelCb.pointLightCount + 1, pixelCbHeap);
	auto lightPassCBHeapIndex = frame->CopyAllocate(pixelCb.pointLightCount, cbHeap, constBufferIndex);
	constBufferIndex = index;

	DirShadowBuffer cb;
	cb.shadowView = shadowViewTransposed;
	cb.shadowProjection = shadowProjTransposed;
	int count = 0;
	for (auto e : entities)
	{
		if (!e->CastsShadow()) continue;
		cb.world = e->GetWorldMatrixTransposed();
		shadowCBWrapper.CopyData(&cb, sizeof(DirShadowBuffer), count); //0th position taken by Point Shadow Buffer
		//index++;
		count++;
	}

	auto shadowHeapIndex = frame->CopyAllocate(count, shadowCbHeap, 0);
	constBufferIndex = index;


	//Create Skybox CBV
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixTranspose(XMMatrixIdentity()));
	auto skycb = ConstantBuffer
	{
		identity,
		identity,
		camera->GetViewMatrixTransposed(),
		camera->GetProjectionMatrixTransposed()
	};
	cbWrapper.CopyData(&skycb, ConstantBufferSize, constBufferIndex);
	auto skyCBIndex = frame->CopyAllocate(1, cbHeap, constBufferIndex);
	constBufferIndex++;

	pShadowBuffer = {};
	XMFLOAT3 dirs[] = {
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0,1, 0 },
		{ 0, -1, 0 },
		{ 0, 0, 1 },
		{ 0, 0, -1 }
	};

	XMFLOAT3 ups[] =
	{
		{ 0.0f, -1.0f, 0.0 },
		{ 0.0f, -1.0f, 0.0 },
		{ 0.0f, 0.0f, -1.0 },
		{ 0.0f, 0.0f,  1.0 },
		{ 0.0f, -1.0f, 0.0 },
		{ 0.0f, -1.0f, 0.0 }
	};

	auto proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.f, 0.1f, pixelCb.pointLight[0].Range);
	XMFLOAT4X4 pointProj;
	XMStoreFloat4x4(&pointProj, proj);
	//ZeroMemory(&pShadowBuffer, sizeof(PointShadowBuffer));
	//for (int i = 0; i < 6; ++i)
	//{
	//	auto view = XMMatrixLookToLH(XMLoadFloat3(&pixelCb.pointLight[0].Position), XMLoadFloat3(&dirs[i]), XMLoadFloat3(&ups[i]));
	//	XMStoreFloat4x4(&pShadowBuffer.viewProjection[i], XMMatrixTranspose(view * proj));
	//}

	auto world = XMMatrixTranslationFromVector(-XMLoadFloat3(&pixelCb.pointLight[0].Position));
	auto view = XMMatrixRotationY(XM_PI + XM_PIDIV2); // +X
	XMStoreFloat4x4(&pShadowBuffer.viewProjection[0], XMMatrixTranspose(world * view * proj));
	view = XMMatrixRotationY(XM_PIDIV2); //-X
	XMStoreFloat4x4(&pShadowBuffer.viewProjection[1], XMMatrixTranspose(world * view * proj));

	view = XMMatrixRotationX(XM_PIDIV2); //+Y
	XMStoreFloat4x4(&pShadowBuffer.viewProjection[2], XMMatrixTranspose(world * view * proj));
	view = XMMatrixRotationX(XM_PI + XM_PIDIV2); //-Y
	XMStoreFloat4x4(&pShadowBuffer.viewProjection[3], XMMatrixTranspose(world * view * proj));

	XMStoreFloat4x4(&pShadowBuffer.viewProjection[4], XMMatrixTranspose(world * proj)); //+Z
	view = XMMatrixRotationY(XM_PI);
	XMStoreFloat4x4(&pShadowBuffer.viewProjection[5], XMMatrixTranspose(world * view * proj));//-Z

	pointShadowCBWrapper.CopyData(&pShadowBuffer, sizeof(PointShadowBuffer), 0);



	//Create Per Frame CBV
	int PerFrameCBSize = (sizeof(PerFrameConstantBuffer) + 255) & ~255;
	ZeroMemory(&frameCB, sizeof(PerFrameConstantBuffer));
	frameCB = { camera->GetNearZ(), camera->GetFarZ() , XMFLOAT2(pointProj._33, pointProj._43) }; //Projection Constants for DOF and Light Perspective for Point Light
	perFrameCbWrapper.CopyData(&frameCB, PerFrameCBSize, 0);
	auto perFrameCBVHeapIndex = frame->CopyAllocate(1, cbHeap, ConstBufferCount - 1);

	//Assign heap indices to frame heap parameters
	frameHeapParams.GBuffer = currentGBufferIndex;
	frameHeapParams.Entities = entitiesHeapIndex;
	frameHeapParams.PixelCB = pixelCbHeapIndex;
	frameHeapParams.Textures = srvGpuHeapIndex;
	frameHeapParams.LightShapes = lightPassCBHeapIndex;
	frameHeapParams.PerFrameCB = perFrameCBVHeapIndex;
	frameHeapParams.ShadowCB = shadowHeapIndex;
	frameHeapParams.SkyCB = skyCBIndex;
}

CDescriptorHeapWrapper& DeferredRenderer::GetSRVHeap()
{
	return srvHeap;
}

CDescriptorHeapWrapper & DeferredRenderer::GetGBufferHeap()
{
	return gBufferHeap;
}

CDescriptorHeapWrapper & DeferredRenderer::GetCBHeap()
{
	return cbHeap;
}

void DeferredRenderer::CreateCB()
{
	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = 1024 * 128;
	resourceDesc.Height = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&worldViewCB));

	resourceDesc.Width = 1024 * 1024 * 2;
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&lightCB));

	resourceDesc.Width = 1024 * 128;
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&perFrameCB));

}

void DeferredRenderer::CreateViews()
{
	gBufferHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 32);// , true);

	int numCBsForNow = ConstBufferCount;
	cbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numCBsForNow);
	pixelCbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numCBsForNow);

	//Camera CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
	descBuffer.BufferLocation = worldViewCB->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = ConstantBufferSize;


	for (int i = 0; i < numCBsForNow - 1; ++i)
	{
		descBuffer.BufferLocation = worldViewCB->GetGPUVirtualAddress() + i * ConstantBufferSize;
		device->CreateConstantBufferView(&descBuffer, cbHeap.handleCPU(i));
	}

	//Light CBV
	descBuffer.BufferLocation = lightCB->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = PixelConstantBufferSize;
	for (int i = 0; i < numCBsForNow; ++i)
	{
		descBuffer.BufferLocation = lightCB->GetGPUVirtualAddress() + i * PixelConstantBufferSize;
		device->CreateConstantBufferView(&descBuffer, pixelCbHeap.handleCPU(i));
	}

	int perFrameCBSize = (sizeof(PerFrameConstantBuffer) + 255) & ~255;
	descBuffer.BufferLocation = perFrameCB->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = (UINT)perFrameCBSize;
	device->CreateConstantBufferView(&descBuffer, cbHeap.handleCPU(ConstBufferCount - 1));

	perFrameCbWrapper.Initialize(perFrameCB, perFrameCBSize);
	cbWrapper.Initialize(worldViewCB, ConstantBufferSize);
	pixelCbWrapper.Initialize(lightCB, PixelConstantBufferSize);

}

void DeferredRenderer::CreatePSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));
	descPipelineState.VS = ShaderManager::LoadShader(L"DefaultVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"DeferredPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = numRTV + 1;
	descPipelineState.RTVFormats[0] = mRtvFormat[0];
	descPipelineState.RTVFormats[1] = mRtvFormat[1];
	descPipelineState.RTVFormats[2] = mRtvFormat[2];
	descPipelineState.RTVFormats[3] = mRtvFormat[3];
	descPipelineState.RTVFormats[4] = mRtvFormat[4];
	descPipelineState.RTVFormats[7] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descPipelineState.DSVFormat = mDsvFormat;
	descPipelineState.SampleDesc.Count = 1;
	deferredPSO = sysRM->CreatePSO(StringID("deferredPSO"), descPipelineState);

	descPipelineState.VS = ShaderManager::LoadShader(L"InstancedDefaultVS.cso");
	descPipelineState.InputLayout.pInputElementDescs = InputLayout::InstanceDefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::InstanceDefaultLayout);
	sysRM->CreatePSO(StringID("instancedDeferredPSO"), descPipelineState);
}


void DeferredRenderer::CreateSkyboxPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));
	auto depthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilState.DepthEnable = true;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	auto rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerState.DepthClipEnable = true;
	rasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	descPipelineState.VS = ShaderManager::LoadShader(L"SkyboxVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"SkyboxPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = depthStencilState;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = rasterizerState;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descPipelineState.SampleDesc.Count = 1;
	descPipelineState.DSVFormat = mDsvFormat;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&skyboxPSO));
}

void DeferredRenderer::CreateLightPassPSO()
{
	auto blendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendState.AlphaToCoverageEnable = false;
	blendState.IndependentBlendEnable = false;

	blendState.RenderTarget[0].BlendEnable = true;
	blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"ScreenQuadVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"AmbientPassPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = nullptr;
	descPipelineState.InputLayout.NumElements = 0;// _countof(inputLayout);
	descPipelineState.pRootSignature = rootSignature;
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

	auto ambientPassPSO = sysRM->CreatePSO(StringID("ambientPass"), descPipelineState);

	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"ScreenQuadVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"LightPassPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = nullptr;
	descPipelineState.InputLayout.NumElements = 0;// _countof(inputLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.DepthStencilState.DepthEnable = false;
	descPipelineState.BlendState = blendState;
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState.DepthClipEnable = false;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descPipelineState.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&dirLightPassPSO));

	auto rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizer.CullMode = D3D12_CULL_MODE_FRONT; // Front culling for point light
	rasterizer.DepthClipEnable = false;
	descPipelineState.VS = ShaderManager::LoadShader(L"LightShapeVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"LightShapePassPS.cso");
	descPipelineState.BlendState = blendState;
	descPipelineState.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	descPipelineState.RasterizerState = rasterizer;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	
	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&shapeLightPassPSO));

}

void DeferredRenderer::CreateScreenQuadPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"ScreenQuadVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"ScreenQuadPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = nullptr;
	descPipelineState.InputLayout.NumElements = 0;// _countof(inputLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.DepthStencilState.DepthEnable = false;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState.DepthClipEnable = false;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	descPipelineState.SampleDesc.Count = 1;

	screenQuadPSO = sysRM->CreatePSO(StringID("screenQuad"), descPipelineState);
}



void DeferredRenderer::CreateRTV()
{
	gRTVHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, numRTV + 1); // +1 for shadow pos texture
	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = viewportWidth;
	resourceDesc.Height = viewportHeight;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_CLEAR_VALUE clearVal;
	clearVal.Color[0] = mClearColor[0];
	clearVal.Color[1] = mClearColor[1];
	clearVal.Color[2] = mClearColor[2];
	clearVal.Color[3] = mClearColor[3];

	for (int i = 0; i < numRTV; i++) {
		resourceDesc.Format = mRtvFormat[i];
		clearVal.Format = mRtvFormat[i];
		device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal, IID_PPV_ARGS(&gBufferTextures[i]));
	}

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, &clearVal, IID_PPV_ARGS(&postProcessTexture));

	D3D12_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;

	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < numRTV; i++) {
		desc.Format = mRtvFormat[i];
		device->CreateRenderTargetView(gBufferTextures[i], &desc, gRTVHeap.handleCPU(i));
	}

	//Create SRV for RTs
	D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;

	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Texture2D.MipLevels = resourceDesc.MipLevels;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Format = resourceDesc.Format;
	descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 256);

	for (int i = 0; i < numRTV; i++)
	{
		descSRV.Format = mRtvFormat[i];
		device->CreateShaderResourceView(gBufferTextures[i], &descSRV, gBufferHeap.handleCPU(i));
		gBufferTextureVector.push_back(new Texture(this, device, gBufferTextures[i], i, TextureTypeSRV, &gBufferHeap));
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	UAVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	UAVDesc.Buffer.NumElements = 1;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	descSRV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	device->CreateUnorderedAccessView(gBufferTextures[RTV_ORDER_QUAD], nullptr, &UAVDesc, srvHeap.handleCPU(srvHeapIndex));
	resultUAV = new Texture(this, device, gBufferTextures[RTV_ORDER_QUAD], srvHeapIndex, TextureTypeUAV);
	srvHeapIndex++;

	device->CreateShaderResourceView(gBufferTextures[RTV_ORDER_QUAD], &descSRV, srvHeap.handleCPU(srvHeapIndex));
	resultSRV = new Texture(this, device, gBufferTextures[RTV_ORDER_QUAD], srvHeapIndex, TextureTypeSRV);
	srvHeapIndex++;

	device->CreateUnorderedAccessView(postProcessTexture, nullptr, &UAVDesc, srvHeap.handleCPU(srvHeapIndex));
	postProcessUAV = new Texture(this, device, postProcessTexture, srvHeapIndex, TextureTypeUAV);
	srvHeapIndex++;

	device->CreateShaderResourceView(postProcessTexture, &descSRV, srvHeap.handleCPU(srvHeapIndex));
	postProcessSRV = new Texture(this, device, postProcessTexture, srvHeapIndex, TextureTypeSRV);
	srvHeapIndex++;

	textureVector = { resultUAV , resultSRV, postProcessUAV, postProcessSRV };
}

void DeferredRenderer::CreateDSV()
{
	dsvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 8); //Also holds shadow map depth stencil

	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = mDsvFormat;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = viewportWidth;
	resourceDesc.Height = viewportHeight;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearVal;
	clearVal = { mDsvFormat , mClearDepth };
	depthStencilTexture = sysRM->CreateResource(StringID("depthStencil"), resourceDesc, ResourceTypeTexture, &clearVal, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Texture2D.MipSlice = 0;
	desc.Format = resourceDesc.Format;
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(depthStencilTexture, &desc, dsvHeap.handleCPU(0));

	D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;

	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Texture2D.MipLevels = resourceDesc.MipLevels;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	device->CreateShaderResourceView(depthStencilTexture, &descSRV, gBufferHeap.handleCPU(RTV_ORDER_COUNT));

	auto heapIndex = SetSRV(depthStencilTexture, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	gDepthSRV = std::make_unique<Texture>(this, device, depthStencilTexture, heapIndex, TextureTypeSRV);
}

void DeferredRenderer::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range[4];
	//view dependent CBV
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//light dependent CBV
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//G-Buffer inputs
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);
	//per frame CBV
	range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	//range[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[4];
	rootParameters[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_ALL);
	//rootParameters[4].InitAsDescriptorTable(1, &range[4], D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(4, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[2];
	StaticSamplers[0].Init(0, D3D12_FILTER_ANISOTROPIC);
	StaticSamplers[1].Init(1, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.f, 16u, D3D12_COMPARISON_FUNC_LESS_EQUAL);
	descRootSignature.NumStaticSamplers = 2;
	descRootSignature.pStaticSamplers = StaticSamplers;

	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	auto hr = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);

	device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

}

void DeferredRenderer::CreateShadowBuffers()
{
	shadowRTVHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 6);//shadowMapCount);
	shadowDSVHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 6);// shadowMapCount);
	shadowResHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 6);
	shadowCbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ConstBufferCount);
	pointShadowCbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	//shadowPosHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shadowMapCount); // Every shadow map requires a shadow pos texture
	int shadowPosPointTextureIndex = numRTV + 7;
	int shadowMapPointTextureIndex = numRTV + 6;

	int shadowPosTextureIndex = numRTV + 5;
	int shadowMapTextureIndex = numRTV + 4; //// numRTV + 1,2,3 taken up by IBL textures

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));
	auto shadowMapFormat = DXGI_FORMAT_D32_FLOAT;
	descPipelineState.VS = ShaderManager::LoadShader(L"ShadowVS.cso");
	descPipelineState.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.DepthStencilState.DepthEnable = true;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	descPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	descPipelineState.RasterizerState.AntialiasedLineEnable = TRUE;
	descPipelineState.RasterizerState.DepthClipEnable = true;
	descPipelineState.RasterizerState.DepthBias = 1000;
	descPipelineState.RasterizerState.DepthBiasClamp = 0.f;
	descPipelineState.RasterizerState.SlopeScaledDepthBias = 5.f;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 0;
	//descPipelineState.RTVFormats[0] = mDsvFormat;
	descPipelineState.DSVFormat = shadowMapFormat;
	descPipelineState.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&shadowMapDirLightPSO));

	descPipelineState.InputLayout.pInputElementDescs = InputLayout::InstanceDefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::InstanceDefaultLayout);
	descPipelineState.VS = ShaderManager::LoadShader(L"ShadowInstancedVS.cso");

	sysRM->CreatePSO(StringID("shadowInstancedDirLightPSO"), descPipelineState);

	descPipelineState.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	descPipelineState.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	descPipelineState.VS = ShaderManager::LoadShader(L"ShadowGenVS.cso");
	descPipelineState.GS = ShaderManager::LoadShader(L"ShadowGS.cso");
	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&shadowMapPointLightPSO));

	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = shadowMapFormat;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = shadowMapSize;
	resourceDesc.Height = shadowMapSize;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearVal;
	clearVal = { shadowMapFormat , mClearDepth };

	D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;

	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Texture2D.MipLevels = resourceDesc.MipLevels;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Format = DXGI_FORMAT_R32_FLOAT;
	descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shadowMapTexture = sysRM->CreateResource(StringID("shadowMap"), resourceDesc, ResourceTypeTexture, &clearVal, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&shadowMapTexture));

	//for (int i = 0; i < MaxDirLights; ++i)
	//{
	//	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &clearVal, IID_PPV_ARGS(&shadowMaps[i]));
	//}

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.DepthOrArraySize = 6;
	resourceDesc.Width = 1024;
	resourceDesc.Height = 1024;
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&shadowMapPointTexture));

	//for (int i = MaxDirLights; i < MaxDirLights + MaxPointLights; ++i)
	//{
	//	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &clearVal, IID_PPV_ARGS(&shadowMaps[i]));
	//}

	D3D12_RESOURCE_DESC cbResDesc;
	ZeroMemory(&cbResDesc, sizeof(resourceDesc));
	cbResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResDesc.Alignment = 0;
	cbResDesc.SampleDesc.Count = 1;
	cbResDesc.SampleDesc.Quality = 0;
	cbResDesc.MipLevels = 1;
	cbResDesc.Format = DXGI_FORMAT_UNKNOWN;
	cbResDesc.DepthOrArraySize = 1;
	cbResDesc.Width = ((sizeof(DirShadowBuffer) + 255) & ~255) * (ConstBufferCount + 1);
	cbResDesc.Height = 1;
	cbResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	CD3DX12_HEAP_PROPERTIES uheapProperty(D3D12_HEAP_TYPE_UPLOAD);
	device->CreateCommittedResource(&uheapProperty, D3D12_HEAP_FLAG_NONE, &cbResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&shadowCB));
	cbResDesc.Width = ((sizeof(PointShadowBuffer) + 255) & ~255);
	device->CreateCommittedResource(&uheapProperty, D3D12_HEAP_FLAG_NONE, &cbResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pointShadowCB));
	const static int size = (sizeof(PointShadowBuffer) + 255) & ~255;

	D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
	descBuffer.BufferLocation = pointShadowCB->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = size;
	device->CreateConstantBufferView(&descBuffer, pointShadowCbHeap.hCPUHeapStart);
	pointShadowCBWrapper.Initialize(pointShadowCB, size);

	const int dirShadowBufferSize = ((sizeof(DirShadowBuffer) + 255) & ~255);
	for (int i = 0; i < ConstBufferCount; ++i)
	{
		descBuffer.BufferLocation = shadowCB->GetGPUVirtualAddress() + i * dirShadowBufferSize;
		descBuffer.SizeInBytes = dirShadowBufferSize;
		device->CreateConstantBufferView(&descBuffer, shadowCbHeap.handleCPU(i));
	}
	shadowCBWrapper.Initialize(shadowCB, dirShadowBufferSize);
	

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Texture2D.MipSlice = 0;
	desc.Format = resourceDesc.Format;
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(shadowMapTexture, &desc, shadowDSVHeap.handleCPU(0)); // 0 is the main depth target
	device->CreateShaderResourceView(shadowMapTexture, &descSRV, gBufferHeap.handleCPU(shadowMapTextureIndex));

	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.ArraySize = 6u;
	desc.Texture2DArray.FirstArraySlice = 0u;
	device->CreateDepthStencilView(shadowMapPointTexture, &desc, shadowDSVHeap.handleCPU(1));

	descSRV.TextureCube.MipLevels = 1;
	descSRV.TextureCube.MostDetailedMip = 0;
	descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	device->CreateShaderResourceView(shadowMapPointTexture, &descSRV, gBufferHeap.handleCPU(shadowMapPointTextureIndex));

	auto shadowPosFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	D3D12_CLEAR_VALUE clearVal2;
	clearVal2.Color[0] = mClearColor[0];
	clearVal2.Color[1] = mClearColor[1];
	clearVal2.Color[2] = mClearColor[2];
	clearVal2.Color[3] = mClearColor[3];
	clearVal2.Format = shadowPosFormat;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = viewportWidth;
	resourceDesc.Height = viewportHeight;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resourceDesc.Format = shadowPosFormat;
	shadowPosTexture = sysRM->CreateResource(StringID("shadowPos"), resourceDesc, ResourceTypeTexture, &clearVal2, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal2, IID_PPV_ARGS(&shadowPosPointTexture));
	CreateShaderResourceView(device, shadowPosTexture, gBufferHeap.handleCPU(shadowPosTextureIndex));
	CreateShaderResourceView(device, shadowPosPointTexture, gBufferHeap.handleCPU(shadowPosPointTextureIndex));

	D3D12_RENDER_TARGET_VIEW_DESC descRT;
	ZeroMemory(&descRT, sizeof(descRT));
	descRT.Texture2D.MipSlice = 0;
	descRT.Texture2D.PlaneSlice = 0;
	descRT.Format = shadowPosFormat;
	descRT.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(shadowPosTexture, &descRT, gRTVHeap.handleCPU(numRTV));

	samplerHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1, true);
	D3D12_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowSampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	shadowSampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	shadowSampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSampler(&shadowSampDesc, samplerHeap.hCPUHeapStart);
}

void DeferredRenderer::CreateSelectionFilterBuffers()
{
	//Create PSO (DepthOnlyVS) without Pixel Shader, draw only depth
	//Create Depth Texture along with SRV
	//Send Depth Texture to edge detection Compute Shader along with final color texture which returns an texture with edge colored
	//https://gamedev.stackexchange.com/questions/159585/sobel-edge-detection-on-depth-texture

	pRTVHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 4);

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"DepthOnlyVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"SelectionPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = inputLayout;
	descPipelineState.InputLayout.NumElements = _countof(inputLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.DepthStencilState.DepthEnable = true;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descPipelineState.DSVFormat = mDsvFormat;
	descPipelineState.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&selectionFilterPSO));

	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = mDsvFormat;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Width = viewportWidth;
	resourceDesc.Height = viewportHeight;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearVal;
	clearVal = { mDsvFormat , mClearDepth };
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &clearVal, IID_PPV_ARGS(&selectedDepthTexture));

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Texture2D.MipSlice = 0;
	desc.Format = resourceDesc.Format;
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_DSV_FLAG_NONE;

	clearVal.Color[0] = mClearColor[0];
	clearVal.Color[1] = mClearColor[1];
	clearVal.Color[2] = mClearColor[2];
	clearVal.Color[3] = mClearColor[3];
	clearVal.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	resourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal, IID_PPV_ARGS(&selectedOutlineTexture));

	auto heapIndex = SetSRV(selectedOutlineTexture, DXGI_FORMAT_R32G32B32A32_FLOAT);
	selectedOutlineSRV = std::unique_ptr<Texture>(new Texture(this, device, selectedOutlineTexture, heapIndex, TextureTypeSRV));

	D3D12_RENDER_TARGET_VIEW_DESC descRT = {};
	descRT.Texture2D.MipSlice = 0;
	descRT.Texture2D.PlaneSlice = 0;
	descRT.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descRT.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(selectedOutlineTexture, &descRT, pRTVHeap.handleCPU(0));

	device->CreateDepthStencilView(selectedDepthTexture, &desc, dsvHeap.handleCPU(1)); // 0 is the main depth target, 1 is the shadow map
	heapIndex = SetSRV(selectedDepthTexture, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	selectedDepthBufferSRV = std::unique_ptr<Texture>(new Texture(this, device, selectedDepthTexture, heapIndex, TextureTypeSRV));
}

DeferredRenderer::~DeferredRenderer()
{
	for (int i = 0; i < numRTV; ++i)
		gBufferTextures[numRTV]->Release();
	selectedDepthTexture->Release();
	selectedOutlineTexture->Release();
	rootSignature->Release();
	shadowMapPointTexture->Release();
	shadowPosPointTexture->Release();
	postProcessTexture->Release();
	delete resultUAV;
	delete resultSRV;
	delete postProcessSRV;
	delete postProcessUAV;
	for (auto& t : gBufferTextureVector)
	{
		delete t;
	}

	dirLightPassPSO->Release();
	shapeLightPassPSO->Release();
	skyboxPSO->Release();
	shadowMapDirLightPSO->Release();
	shadowMapPointLightPSO->Release();
	selectionFilterPSO->Release();

	perFrameCB->Release();
	lightCB->Release();
	worldViewCB->Release();
	shadowCB->Release();
	pointShadowCB->Release();
	delete sphereMesh;
	delete cubeMesh;
}
