#include "DeferredRenderer.h"
#include "ShaderManager.h"
#include "Vertex.h"

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
	srvDesc.Texture2D.MipLevels = 5;
	device->CreateShaderResourceView(prefilterTextureCube, &srvDesc, gBufferHeap.handleCPU(prefilterIndex));
}

Texture * DeferredRenderer::GetResultUAV()
{
	return resultUAV;
}

Texture * DeferredRenderer::GetResultSRV()
{
	return resultSRV;
}

void DeferredRenderer::Initialize(ID3D12GraphicsCommandList* command)
{
	CreateCB();
	CreateViews();
	CreateRootSignature();

	CreatePSO();
	CreateSkyboxPSO();
	CreateLightPassPSO();
	CreateScreenQuadPSO();
	CreatePrefilterEnvironmentPSO();

	CreateRTV();
	CreateDSV();
	CreatePrefilterResources(command);

	sphereMesh = new Mesh("../../Assets/sphere.obj", device, command);
	cubeMesh = new Mesh("../../Assets/cube.obj", device, command);
}

void DeferredRenderer::GeneratePreFilterEnvironmentMap(ID3D12GraphicsCommandList* command, int envTextureIndex)
{
	//	TODO: Needs depth stencil state
	// TODO: Separate Initialization code for Prefilter Env Map
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMFLOAT4X4 camViewMatrix;
	XMFLOAT4X4 camProjMatrix;
	XMVECTOR tar[] = { XMVectorSet(1, 0, 0, 0), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 0, -1, 0) };
	XMVECTOR up[] = { XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 0, -1, 0), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 1, 0, 0) };
	int maxMipLevels = 5;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 64.f;
	viewport.Height = 64.f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = 64L;
	scissorRect.bottom = 64L;


	command->SetGraphicsRootSignature(rootSignature);
	command->SetPipelineState(prefilterEnvMapPSO);
	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(prefilterTexture, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	for (int mip = 0; mip < maxMipLevels; ++mip)
	{
		float mipWidth = 256 * pow(0.5f, mip);
		float mipHeight = 256 * pow(0.5f, mip);

		viewport.Width = mipWidth;
		viewport.Height = mipHeight;

		scissorRect.bottom = (LONG)mipHeight;
		scissorRect.right = (LONG)mipWidth;

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		constBufferIndex = 0;
		for (int i = 0; i < 6; ++i)
		{
			// Camera
			XMVECTOR dir = XMVector3Rotate(tar[i], XMQuaternionIdentity());
			XMMATRIX view = DirectX::XMMatrixLookToLH(XMLoadFloat3(&position), dir, up[i]);
			XMStoreFloat4x4(&camViewMatrix, DirectX::XMMatrixTranspose(view));

			XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.5f * XM_PI, 1.0f, 0.1f, 100.0f);
			XMStoreFloat4x4(&camProjMatrix, DirectX::XMMatrixTranspose(P));
			XMFLOAT4X4 identity;
			XMStoreFloat4x4(&identity, XMMatrixTranspose(XMMatrixIdentity()));

			command->OMSetRenderTargets(1, &prefilterRTVHeap.handleCPU(i + mip * maxMipLevels), true, &dsvHeap.handleCPU(0));
			command->RSSetViewports(1, &viewport);
			command->RSSetScissorRects(1, &scissorRect);
			command->ClearRenderTargetView(prefilterRTVHeap.handleCPU(i + mip * maxMipLevels), mClearColor, 0, nullptr);

			ID3D12DescriptorHeap* ppSrvHeaps[] = { srvHeap.pDescriptorHeap.Get() };
			ID3D12DescriptorHeap* ppCbHeaps[] = { cbHeap.pDescriptorHeap.Get() };

			auto cb = ConstantBuffer{
				identity,
				identity,
				camViewMatrix,
				camProjMatrix
			};

			cbWrapper.CopyData(&cb, ConstantBufferSize, i);
			command->SetDescriptorHeaps(1, ppCbHeaps);
			command->SetGraphicsRootDescriptorTable(0, cbHeap.handleGPU(i));
			command->SetDescriptorHeaps(1, ppSrvHeaps);
			command->SetGraphicsRootDescriptorTable(2, srvHeap.handleGPU(envTextureIndex));
			Draw(cubeMesh, cb, command);
			constBufferIndex++;
		}
	}

	//command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(prefilterTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	//preFilterRTVHeap.pDescriptorHeap->Release();
	//prefilterTexture->Release();
}

void DeferredRenderer::SetGBUfferPSO(ID3D12GraphicsCommandList* command, Camera* camera, const PixelConstantBuffer& pixelCb)
{
	this->camera = camera;
	this->entities = entities;
	ID3D12DescriptorHeap* ppHeaps[] = { pixelCbHeap.pDescriptorHeap.Get() };

	command->SetPipelineState(deferredPSO);
	for (int i = 0; i < numRTV; i++)
		command->ClearRenderTargetView(rtvHeap.handleCPU(i), mClearColor, 0, nullptr);

	command->ClearDepthStencilView(dsvHeap.hCPUHeapStart, D3D12_CLEAR_FLAG_DEPTH, mClearDepth, 0xff, 0, nullptr);
	command->OMSetRenderTargets(numRTV, &rtvHeap.hCPUHeapStart, true, &dsvHeap.hCPUHeapStart);
	command->SetGraphicsRootSignature(rootSignature);

	command->SetDescriptorHeaps(1, ppHeaps);
	command->SetGraphicsRootDescriptorTable(1, pixelCbHeap.handleGPU(0));

	pixelCbWrapper.CopyData((void*)&pixelCb, sizeof(PixelConstantBuffer), 0);
}

void DeferredRenderer::SetLightPassPSO(ID3D12GraphicsCommandList * command, const PixelConstantBuffer & pixelCb)
{

	/*for (int i = 0; i < numRTV; i++)
		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));*/
		//command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_QUAD], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	command->SetPipelineState(dirLightPassPSO);

	//command->ClearRenderTargetView(rtvHeap.handleCPU(RTV_ORDER_QUAD), mClearColor, 0, nullptr);
	command->OMSetRenderTargets(1, &rtvHeap.handleCPU(RTV_ORDER_QUAD), true, nullptr);

	ID3D12DescriptorHeap* ppHeap2[] = { pixelCbHeap.pDescriptorHeap.Get() };
	command->SetDescriptorHeaps(1, ppHeap2);
	command->SetGraphicsRootDescriptorTable(1, pixelCbHeap.handleGPU(0));
	ID3D12DescriptorHeap* ppHeaps[] = { gBufferHeap.pDescriptorHeap.Get() };
	command->SetDescriptorHeaps(1, ppHeaps);
	command->SetGraphicsRootDescriptorTable(2, gBufferHeap.handleGPU(0));
}

void DeferredRenderer::SetLightShapePassPSO(ID3D12GraphicsCommandList * command, const PixelConstantBuffer & pixelCb)
{
	for (int i = 0; i < numRTV; i++)
		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));

	// (numRTV - 2)th texture and RTV is being used to store the light shape pass
	command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_LIGHTSHAPE], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	command->ClearRenderTargetView(rtvHeap.handleCPU(RTV_ORDER_LIGHTSHAPE), mClearColor, 0, nullptr);
	command->OMSetRenderTargets(1, &rtvHeap.handleCPU(RTV_ORDER_LIGHTSHAPE), true, nullptr);
	command->SetPipelineState(shapeLightPassPSO);

	ID3D12DescriptorHeap* ppHeap2[] = { pixelCbHeap.pDescriptorHeap.Get() };
	command->SetDescriptorHeaps(1, ppHeap2);
	command->SetGraphicsRootDescriptorTable(1, pixelCbHeap.handleGPU(0));
	ID3D12DescriptorHeap* ppHeaps[] = { gBufferHeap.pDescriptorHeap.Get() };
	command->SetDescriptorHeaps(1, ppHeaps);
	command->SetGraphicsRootDescriptorTable(2, gBufferHeap.handleGPU(0));

}

void DeferredRenderer::Draw(ID3D12GraphicsCommandList* commandList, std::vector<Entity*> entities)
{
	int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBuffer) + 255) & ~255;
	int index = 0;
	ID3D12DescriptorHeap* ppHeaps[] = { cbHeap.pDescriptorHeap.Get() };
	ID3D12DescriptorHeap* ppSrvHeaps[] = { srvHeap.pDescriptorHeap.Get() };
	//ConstantBuffer cb;
	for (auto e : entities)
	{
		commandList->SetDescriptorHeaps(1, ppSrvHeaps);
		commandList->SetGraphicsRootDescriptorTable(2, e->GetMaterial()->GetGPUDescriptorHandle()); //Set start of material texture in root descriptor

		commandList->SetDescriptorHeaps(1, ppHeaps);
		auto cb = ConstantBuffer
		{
			e->GetWorldViewProjectionTransposed(camera->GetProjectionMatrix(), camera->GetViewMatrix()),
			e->GetWorldMatrixTransposed(),
			camera->GetViewMatrixTransposed(),
			camera->GetProjectionMatrixTransposed()
		};
		cbWrapper.CopyData(&cb, ConstantBufferPerObjectAlignedSize, index);
		commandList->SetGraphicsRootDescriptorTable(0, cbHeap.handleGPU(index));

		Draw(e->GetMesh(), cb, commandList);
		index++;
	}
	constBufferIndex += index;
}

void DeferredRenderer::DrawSkybox(ID3D12GraphicsCommandList * commandList, Texture* skybox)
{
	int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBuffer) + 255) & ~255;
	commandList->SetPipelineState(skyboxPSO);
	commandList->OMSetRenderTargets(1, &rtvHeap.handleCPU(RTV_ORDER_QUAD), true, &dsvHeap.hCPUHeapStart);
	ID3D12DescriptorHeap* ppHeaps[] = { cbHeap.pDescriptorHeap.Get() };
	ID3D12DescriptorHeap* ppSrvHeaps[] = { srvHeap.pDescriptorHeap.Get() };
	XMFLOAT4X4 identity;
	XMStoreFloat4x4(&identity, XMMatrixTranspose(XMMatrixIdentity()));
	auto cb = ConstantBuffer
	{
		identity,
		identity,
		camera->GetViewMatrixTransposed(),
		camera->GetProjectionMatrixTransposed()
	};

	commandList->SetDescriptorHeaps(1, ppSrvHeaps);
	commandList->SetGraphicsRootDescriptorTable(2, skybox->GetGPUDescriptorHandle()); //Set skybox texture
	commandList->SetDescriptorHeaps(1, ppHeaps);

	cbWrapper.CopyData(&cb, ConstantBufferPerObjectAlignedSize, constBufferIndex);
	commandList->SetGraphicsRootDescriptorTable(0, cbHeap.handleGPU(constBufferIndex)); // set constant buffer with view and projection matrices
	constBufferIndex++;
	Draw(cubeMesh, cb, commandList);
}

void DeferredRenderer::DrawLightPass(ID3D12GraphicsCommandList * commandList)
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

void DeferredRenderer::DrawLightShapePass(ID3D12GraphicsCommandList * commandList, const PixelConstantBuffer & pixelCb)
{
	int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBuffer) + 255) & ~255;
	int index = 0;
	ID3D12DescriptorHeap* ppHeaps[] = { cbHeap.pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, ppHeaps);
	Entity e;
	e.SetMesh(sphereMesh);
	e.SetPosition(pixelCb.pointLight.Position);
	float range = pixelCb.pointLight.Range;
	e.SetScale(XMFLOAT3(range, range, range));
	auto cb = ConstantBuffer{ e.GetWorldViewProjectionTransposed(camera->GetProjectionMatrix(), camera->GetViewMatrix()), e.GetWorldMatrixTransposed() };
	cbWrapper.CopyData(&cb, ConstantBufferPerObjectAlignedSize, constBufferIndex);
	commandList->SetGraphicsRootDescriptorTable(0, cbHeap.handleGPU(constBufferIndex));
	Draw(e.GetMesh(), cb, commandList);
	constBufferIndex++;

	//numRTV - 2 is the lightshape pass RTV
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_LIGHTSHAPE], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void DeferredRenderer::DrawResult(ID3D12GraphicsCommandList* commandList, D3D12_CPU_DESCRIPTOR_HANDLE & rtvHandle)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[RTV_ORDER_QUAD], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	commandList->ClearRenderTargetView(rtvHandle, mClearColor, 0, nullptr);
	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	commandList->SetPipelineState(screenQuadPSO);
	ID3D12DescriptorHeap* ppHeaps[] = { gBufferHeap.pDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(2, gBufferHeap.handleGPU(RTV_ORDER_QUAD));
	DrawLightPass(commandList); // Draws full screen quad with null vertex buffer.
}

void DeferredRenderer::Draw(Mesh * m, const ConstantBuffer & cb, ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m->GetVertexBufferView());
	commandList->IASetIndexBuffer(&m->GetIndexBufferView());
	commandList->DrawIndexedInstanced(m->GetIndexCount(), 1, 0, 0, 0);
}

void DeferredRenderer::UpdateConstantBuffer(const PixelConstantBuffer & pixelBuffer, ID3D12GraphicsCommandList* command)
{
	void* mapped = nullptr;
	lightCB->Map(0, nullptr, &mapped);
	if (mapped == nullptr) return;
	memcpy(mapped, &pixelBuffer, sizeof(PixelConstantBuffer));
	lightCB->Unmap(0, nullptr);
}

void DeferredRenderer::UpdateConstantBufferPerObject(ConstantBuffer& buffer, int index)
{
	int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBuffer) + 255) & ~255;
	UINT8 *cbvGPUAddress;
	CD3DX12_RANGE readRange(0, 0);
	worldViewCB->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress));
	memcpy(cbvGPUAddress + ConstantBufferPerObjectAlignedSize * index, &buffer, sizeof(ConstantBuffer));
}

CDescriptorHeapWrapper& DeferredRenderer::GetSRVHeap()
{
	return srvHeap;
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

	resourceDesc.Width = 1024 * 128;
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&lightCB));
}

void DeferredRenderer::CreateViews()
{
	gBufferHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 32, true);

	const int numCBsForNow = 32;
	cbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numCBsForNow, true);
	pixelCbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numCBsForNow, true);

	//Camera CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
	descBuffer.BufferLocation = worldViewCB->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = ConstantBufferSize;


	for (int i = 0; i < numCBsForNow; ++i)
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

	cbWrapper.Initialize(worldViewCB, ConstantBufferSize);
	pixelCbWrapper.Initialize(lightCB, PixelConstantBufferSize);

}

void DeferredRenderer::CreatePSO()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));
	descPipelineState.VS = ShaderManager::LoadShader(L"DefaultVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"DeferredPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = inputLayout;
	descPipelineState.InputLayout.NumElements = _countof(inputLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = numRTV;
	descPipelineState.RTVFormats[0] = mRtvFormat[0];
	descPipelineState.RTVFormats[1] = mRtvFormat[1];
	descPipelineState.RTVFormats[2] = mRtvFormat[2];
	descPipelineState.RTVFormats[3] = mRtvFormat[3];
	descPipelineState.RTVFormats[4] = mRtvFormat[4];
	descPipelineState.DSVFormat = mDsvFormat;
	descPipelineState.SampleDesc.Count = 1;
	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&deferredPSO));
}

void DeferredRenderer::CreatePrefilterEnvironmentPSO()
{
	// Almost same as skybox PSO except the pixel shader is for prefilter env map.
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

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
	descPipelineState.PS = ShaderManager::LoadShader(L"PrefilterEnvMapPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = inputLayout;
	descPipelineState.InputLayout.NumElements = _countof(inputLayout);
	descPipelineState.pRootSignature = rootSignature;
	descPipelineState.DepthStencilState = depthStencilState;
	descPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPipelineState.RasterizerState = rasterizerState;
	descPipelineState.SampleMask = UINT_MAX;
	descPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPipelineState.NumRenderTargets = 1;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	descPipelineState.SampleDesc.Count = 1;
	descPipelineState.DSVFormat = mDsvFormat;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&prefilterEnvMapPSO));
}

void DeferredRenderer::CreatePrefilterResources(ID3D12GraphicsCommandList* command)
{
	prefilterRTVHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 30);
	int maxMipLevels = 5;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 64.f;
	viewport.Height = 64.f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = 64L;
	scissorRect.bottom = 64L;

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = maxMipLevels;
	resourceDesc.DepthOrArraySize = 6;
	resourceDesc.Width = 256;
	resourceDesc.Height = 256;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = maxMipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	D3D12_CLEAR_VALUE clearVal;
	clearVal.Color[0] = mClearColor[0];
	clearVal.Color[1] = mClearColor[1];
	clearVal.Color[2] = mClearColor[2];
	clearVal.Color[3] = mClearColor[3];
	clearVal.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearVal, IID_PPV_ARGS(&prefilterTexture));

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	int srvIndex = 4 * MATERIAL_COUNT + 4;
	//Need to change the CPU Handle index

	for (int mip = 0; mip < maxMipLevels; ++mip)
	{
		float mipWidth = 256 * pow(0.5f, mip);
		float mipHeight = 256 * pow(0.5f, mip);
		rtvDesc.Texture2DArray.MipSlice = mip;

		viewport.Width = mipWidth;
		viewport.Height = mipHeight;

		scissorRect.bottom = (LONG)mipHeight;
		scissorRect.right = (LONG)mipWidth;
		for (int i = 0; i < 6; ++i)
		{
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			device->CreateRenderTargetView(prefilterTexture, &rtvDesc, prefilterRTVHeap.handleCPU(i + mip * maxMipLevels));
		}
	}

	device->CreateShaderResourceView(prefilterTexture, &srvDesc, srvHeap.handleCPU(srvIndex));
}

void DeferredRenderer::CreateSkyboxPSO()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

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
	descPipelineState.InputLayout.pInputElementDescs = inputLayout;
	descPipelineState.InputLayout.NumElements = _countof(inputLayout);
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
	//D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	//{
	//	/*{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }*/
	//};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"ScreenQuadVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"LightPassPS.cso");
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

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&dirLightPassPSO));

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	auto rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizer.CullMode = D3D12_CULL_MODE_NONE; // Disable culling for point light
	rasterizer.DepthClipEnable = false;
	descPipelineState.VS = ShaderManager::LoadShader(L"LightShapeVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"LightShapePassPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = inputLayout;
	descPipelineState.RasterizerState = rasterizer;
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	descPipelineState.InputLayout.NumElements = _countof(inputLayout);
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

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&screenQuadPSO));
}



void DeferredRenderer::CreateRTV()
{
	rtvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, numRTV);
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

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, &clearVal, IID_PPV_ARGS(&resultTexture));

	D3D12_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;

	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < numRTV; i++) {
		desc.Format = mRtvFormat[i];
		device->CreateRenderTargetView(gBufferTextures[i], &desc, rtvHeap.handleCPU(i));
	}

	//Create SRV for RTs
	D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;

	ZeroMemory(&descSRV, sizeof(descSRV));
	descSRV.Texture2D.MipLevels = resourceDesc.MipLevels;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Format = resourceDesc.Format;
	descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 32, true);

	for (int i = 0; i < numRTV; i++) {
		descSRV.Format = mRtvFormat[i];
		device->CreateShaderResourceView(gBufferTextures[i], &descSRV, gBufferHeap.handleCPU(i));
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
}

void DeferredRenderer::CreateDSV()
{
	dsvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

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

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal, IID_PPV_ARGS(&depthStencilTexture));
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
}

void DeferredRenderer::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range[3];
	//view dependent CBV
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//light dependent CBV
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//G-Buffer inputs
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 12, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);
	//rootParameters[3].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
	StaticSamplers[0].Init(0, D3D12_FILTER_ANISOTROPIC);
	descRootSignature.NumStaticSamplers = 1;
	descRootSignature.pStaticSamplers = StaticSamplers;

	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	auto hr = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);

	device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

}

DeferredRenderer::~DeferredRenderer()
{
	rootSignature->Release();

	rtvHeap.pDescriptorHeap->Release();
	dsvHeap.pDescriptorHeap->Release();
	srvHeap.pDescriptorHeap->Release();
	gBufferHeap.pDescriptorHeap->Release();
	resultTexture->Release();
	delete resultUAV;
	delete resultSRV;

	for (int i = 0; i < numRTV; ++i)
		gBufferTextures[numRTV]->Release();
	depthStencilTexture->Release();

	deferredPSO->Release();
	dirLightPassPSO->Release();
	shapeLightPassPSO->Release();
	skyboxPSO->Release();
	screenQuadPSO->Release();
	prefilterEnvMapPSO->Release();

	prefilterRTVHeap.pDescriptorHeap->Release();
	prefilterTexture->Release();
	lightCB->Release();
	worldViewCB->Release();
	delete sphereMesh;
	delete cubeMesh;
}
