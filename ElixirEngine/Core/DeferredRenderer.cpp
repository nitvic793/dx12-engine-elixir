#include "DeferredRenderer.h"
#include "ShaderManager.h"
#include "Vertex.h"

DeferredRenderer::DeferredRenderer(ID3D12Device* dxDevice, int width, int height) :
	device(dxDevice),
	viewportHeight(height),
	viewportWidth(width)
{
}

void DeferredRenderer::Initialize()
{
	CreateCB();
	CreateViews();
	CreateRootSignature();
	CreatePSO();
	CreateLightPassPSO();

	CreateRTV();
	CreateDSV();
}

void DeferredRenderer::SetGBUfferPSO(ID3D12GraphicsCommandList* command)
{
	ID3D12DescriptorHeap* ppHeaps[1] = { cbvSrvHeap.pDescriptorHeap.Get() };

	command->SetPipelineState(deferredPSO);
	for (int i = 0; i < numRTV; i++)
		command->ClearRenderTargetView(rtvHeap.handleCPU(i), mClearColor, 0, nullptr);

	command->ClearDepthStencilView(dsvHeap.hCPUHeapStart, D3D12_CLEAR_FLAG_DEPTH, mClearDepth, 0xff, 0, nullptr);

	command->OMSetRenderTargets(numRTV, &rtvHeap.hCPUHeapStart, true, &dsvHeap.hCPUHeapStart);
	command->SetDescriptorHeaps(1, ppHeaps);
	command->SetGraphicsRootSignature(rootSignature);
	command->SetGraphicsRootDescriptorTable(0, cbvSrvHeap.handleGPU(0));
	command->SetGraphicsRootDescriptorTable(1, cbvSrvHeap.handleGPU(1));
	//command->SetGraphicsRootDescriptorTable(2, cbvSrvHeap.hGPU(2));
}

void DeferredRenderer::UpdateConstantBuffer(ConstantBuffer & buffer, PixelConstantBuffer & pixelBuffer)
{
	void* mapped = nullptr;
	worldViewCB->Map(0, nullptr, &mapped);
	memcpy(mapped, &buffer, sizeof(ConstantBuffer));
	worldViewCB->Unmap(0, nullptr);

	lightCB->Map(0, nullptr, &mapped);
	memcpy(mapped, &pixelBuffer, sizeof(PixelConstantBuffer));
	lightCB->Unmap(0, nullptr);
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
	resourceDesc.Width = sizeof(ConstantBuffer);
	resourceDesc.Height = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&worldViewCB));

	resourceDesc.Width = sizeof(PixelConstantBuffer);
	device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&lightCB));
}

void DeferredRenderer::CreateViews()
{
	cbvSrvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10, true);
	//Camera CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
	descBuffer.BufferLocation = worldViewCB->GetGPUVirtualAddress();
	//Constant buffer must be larger than 256 bytes
	descBuffer.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;
	device->CreateConstantBufferView(&descBuffer, cbvSrvHeap.handleCPU(0));

	//Light CBV
	descBuffer.BufferLocation = lightCB->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = (sizeof(PixelConstantBuffer) + 255) & ~255;
	device->CreateConstantBufferView(&descBuffer, cbvSrvHeap.handleCPU(1));
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
	descPipelineState.DSVFormat = mDsvFormat;
	descPipelineState.SampleDesc.Count = 1;
	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&deferredPSO));
}

void DeferredRenderer::CreateLightPassPSO()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPipelineState;
	ZeroMemory(&descPipelineState, sizeof(descPipelineState));

	descPipelineState.VS = ShaderManager::LoadShader(L"ScreenQuadVS.cso");
	descPipelineState.PS = ShaderManager::LoadShader(L"LightPassPS.cso");
	descPipelineState.InputLayout.pInputElementDescs = inputLayout;
	descPipelineState.InputLayout.NumElements = _countof(inputLayout);
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

	device->CreateGraphicsPipelineState(&descPipelineState, IID_PPV_ARGS(&lightPassPSO));
}



void DeferredRenderer::CreateRTV()
{
	rtvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3);
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

	//m_srvHeap.Create(g_d3dObjects->GetD3DDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3,true);

	for (int i = 0; i < numRTV; i++) {
		descSRV.Format = mRtvFormat[i];
		device->CreateShaderResourceView(gBufferTextures[i], &descSRV, cbvSrvHeap.handleCPU(i + 2));
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


	device->CreateShaderResourceView(depthStencilTexture, &descSRV, cbvSrvHeap.handleCPU(5));
}

void DeferredRenderer::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range[3];
	//view dependent CBV
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//light dependent CBV
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//G-Buffer inputs
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);

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
	lightPassPSO->Release();

	rtvHeap.pDescriptorHeap->Release();
	dsvHeap.pDescriptorHeap->Release();
	//srvHeap.pDescriptorHeap->Release();
	cbvSrvHeap.pDescriptorHeap->Release();

	lightCB->Release();
	worldViewCB->Release();
}
