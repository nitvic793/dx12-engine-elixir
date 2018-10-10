#include "DeferredRenderer.h"
#include "ShaderManager.h"
#include "Vertex.h"

DeferredRenderer::DeferredRenderer(ID3D12Device* dxDevice, int width, int height) :
	device(dxDevice),
	viewportHeight(height),
	viewportWidth(width)
{
}

void DeferredRenderer::SetSRV(ID3D12Resource* textureSRV, DXGI_FORMAT format, int index)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	//device->CreateShaderResourceView(textureSRV, &srvDesc, srvHeap.pDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart());
	device->CreateShaderResourceView(textureSRV, &srvDesc, srvHeap.handleCPU(index));
}

void DeferredRenderer::Initialize(ID3D12GraphicsCommandList* command)
{
	CreateCB();
	CreateViews();
	CreateRootSignature();
	CreatePSO();
	CreateLightPassPSO();

	CreateRTV();
	CreateDSV();
	sphereMesh = new Mesh("../../Assets/sphere.obj", device, command);
}

void DeferredRenderer::SetGBUfferPSO(ID3D12GraphicsCommandList* command, Camera* camera, const PixelConstantBuffer& pixelCb)
{
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.pDescriptorHeap.Get() };
	this->camera = camera;
	this->entities = entities;
	command->SetPipelineState(deferredPSO);
	for (int i = 0; i < numRTV; i++)
		command->ClearRenderTargetView(rtvHeap.handleCPU(i), mClearColor, 0, nullptr);

	command->ClearDepthStencilView(dsvHeap.hCPUHeapStart, D3D12_CLEAR_FLAG_DEPTH, mClearDepth, 0xff, 0, nullptr);

	command->OMSetRenderTargets(numRTV, &rtvHeap.hCPUHeapStart, true, &dsvHeap.hCPUHeapStart);
	command->SetDescriptorHeaps(1, ppHeaps);
	command->SetGraphicsRootSignature(rootSignature);
	command->SetGraphicsRootDescriptorTable(2, srvHeap.handleGPU(0));
	//command->SetGraphicsRootDescriptorTable(0, cbHeap.handleGPU(0));
	ID3D12DescriptorHeap* ppHeap2[] = { pixelCbHeap.pDescriptorHeap.Get() };
	command->SetDescriptorHeaps(1, ppHeap2);
	command->SetGraphicsRootDescriptorTable(1, pixelCbHeap.handleGPU(0));
	//command->SetGraphicsRootDescriptorTable(2, srvHeap.pDescriptorHeap.Get()->GetGPUDescriptorHandleForHeapStart());

	pixelCbWrapper.CopyData((void*)&pixelCb, sizeof(PixelConstantBuffer), 0);
}

void DeferredRenderer::SetLightPassPSO(ID3D12GraphicsCommandList * command, const PixelConstantBuffer & pixelCb)
{
	
	for (int i = 0; i < numRTV; i++)
		command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gBufferTextures[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	//command->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	command->SetPipelineState(dirLightPassPSO);

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


	command->ClearRenderTargetView(rtvHeap.handleCPU(numRTV-1), mClearColor, 0, nullptr);
	command->OMSetRenderTargets(1, &rtvHeap.handleCPU(numRTV-1), true, nullptr);
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
		commandList->SetGraphicsRootDescriptorTable(2, e->GetMaterial()->GetGPUDescriptorHandle());

		commandList->SetDescriptorHeaps(1, ppHeaps);
		auto cb = ConstantBuffer{ e->GetWorldViewProjectionTransposed(camera->GetProjectionMatrix(), camera->GetViewMatrix()), e->GetWorldMatrixTransposed() };
		cbWrapper.CopyData(&cb, sizeof(ConstantBuffer), index);
		commandList->SetGraphicsRootDescriptorTable(0, cbHeap.handleGPU(index));

		Draw(e->GetMesh(), cb, commandList);
		index++;
	}
	constBufferIndex = index;
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
	cbWrapper.CopyData(&cb, sizeof(ConstantBuffer), constBufferIndex);
	commandList->SetGraphicsRootDescriptorTable(0, cbHeap.handleGPU(constBufferIndex));
	Draw(e.GetMesh(), cb, commandList);
	constBufferIndex++;
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
	resourceDesc.Width = 1024 * 64;
	resourceDesc.Height = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&worldViewCB));

	resourceDesc.Width = 1024 * 64;
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&lightCB));
}

void DeferredRenderer::CreateViews()
{
	gBufferHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10, true);
	cbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10, true);
	pixelCbHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10, true);

	//Camera CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
	descBuffer.BufferLocation = worldViewCB->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = ConstantBufferSize;

	const int numCBsForNow = 5;
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
		{ "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	descPipelineState.DSVFormat = mDsvFormat;
	descPipelineState.SampleDesc.Count = 1;
	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&deferredPSO));
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
	descPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	descPipelineState.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&dirLightPassPSO));

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	auto rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizer.CullMode = D3D12_CULL_MODE_NONE; // Disable culling for point light
	rasterizer.DepthClipEnable = false;
	descPipelineState.VS = ShaderManager::LoadShader(L"LightShapeVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"LightShapePassPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = inputLayout;
	descPipelineState.RasterizerState = rasterizer;
	descPipelineState.InputLayout.NumElements = _countof(inputLayout);
	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&shapeLightPassPSO));

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
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

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


	device->CreateShaderResourceView(depthStencilTexture, &descSRV, gBufferHeap.handleCPU(6));
}

void DeferredRenderer::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range[3];
	//view dependent CBV
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//light dependent CBV
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//G-Buffer inputs
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0);

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

	for (int i = 0; i < numRTV; ++i)
		gBufferTextures[numRTV]->Release();
	depthStencilTexture->Release();

	deferredPSO->Release();
	dirLightPassPSO->Release();
	shapeLightPassPSO->Release();

	rtvHeap.pDescriptorHeap->Release();
	dsvHeap.pDescriptorHeap->Release();
	srvHeap.pDescriptorHeap->Release();
	gBufferHeap.pDescriptorHeap->Release();

	lightCB->Release();
	worldViewCB->Release();
	delete sphereMesh;
}
