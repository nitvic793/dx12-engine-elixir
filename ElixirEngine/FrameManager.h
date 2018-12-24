#pragma once
#include "stdafx.h"
#include "DirectXHelper.h"

class FrameManager
{
	const D3D12_DESCRIPTOR_HEAP_TYPE gpuHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	const uint32_t			DescriptorHeapSize = 1024;
	
	uint32_t				preIndex;
	uint32_t				currentHeapIndex;
	uint32_t				frameStartIndex;
	ID3D12Device*			device;
	CDescriptorHeapWrapper	gpuHeap;

public:
	FrameManager(ID3D12Device* device);
	void						StartFrame();
	void						EndFrame();
	void						CopySimple(uint32_t numDescriptors, CDescriptorHeapWrapper descriptorHeap, uint32_t offset = 0u);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentCPUHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentGPUHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t offset = 0u);
	ID3D12DescriptorHeap*		GetDescriptorHeap();

	~FrameManager();
};

