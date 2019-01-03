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

class SystemResourceManager
{
private:
	static SystemResourceManager * Instance;
protected:
	ID3D12Device* device;
	PSOMap psoMap;
	SystemResourceManager(ID3D12Device* device);
public:
	static SystemResourceManager* CreateInstance(ID3D12Device* device);
	static SystemResourceManager* GetInstance();

	ID3D12PipelineState* CreatePSO(HashID psoID, D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc);
	ID3D12PipelineState* GetPSO(HashID psoID);

	~SystemResourceManager();
};




