#pragma once
#include "stdafx.h"
#include "DirectXHelper.h"
#include "StringHash.h"
#include <wrl/client.h>
#include <unordered_map>

using namespace Microsoft::WRL;

// DirectX 12 specific
typedef std::unordered_map<unsigned int, ID3D12PipelineState*> PSOMap;
typedef std::unordered_map<unsigned int, ID3D12RootSignature*> RootSigMap;
typedef std::unordered_map<unsigned int, ID3D12Resource*> ResourceMap;
typedef std::unordered_map<unsigned int, CDescriptorHeapWrapper> DescriptorHeapMap;
typedef std::unordered_map<unsigned int, ConstantBufferWrapper> ConstantBufferMap;

enum ResourceType
{
	ResourceTypeConstantBuffer,
	ResourceTypeTexture
};

class SystemResourceManager
{
private:
	static SystemResourceManager * Instance;
protected:
	ID3D12Device* device;
	PSOMap psoMap;
	ResourceMap resources;
	SystemResourceManager(ID3D12Device* device);
public:
	static SystemResourceManager* CreateInstance(ID3D12Device* device);
	static SystemResourceManager* GetInstance();

	ID3D12PipelineState*	CreatePSO(HashID psoID, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc);
	ID3D12PipelineState*	GetPSO(HashID psoID);

	ID3D12Resource*			CreateResource(
		HashID resID, 
		D3D12_RESOURCE_DESC resDesc, 
		ResourceType resourceType, 
		D3D12_CLEAR_VALUE* value = nullptr,
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE
	);
	ID3D12Resource*			GetResource(HashID resID);
	void					CopyToCBV(HashID resID);
	~SystemResourceManager();
};




