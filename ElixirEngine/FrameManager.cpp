#include "stdafx.h"
#include "FrameManager.h"

FrameManager::FrameManager(ID3D12Device* device)
{
	this->device = device;
	gpuHeap.Create(device, gpuHeapType, DescriptorHeapSize, true);
	currentHeapIndex = 0;
	preIndex = 0;
	frameStartIndex = 0;
}

void FrameManager::StartFrame()
{
	frameStartIndex = currentHeapIndex;
}

void FrameManager::EndFrame()
{
}

uint32_t FrameManager::CopySimple(uint32_t numDescriptors, CDescriptorHeapWrapper descriptorHeap, uint32_t offset)
{
	auto futureIndex = currentHeapIndex + numDescriptors;
	if (futureIndex >= DescriptorHeapSize)
	{
		currentHeapIndex = 0;
		frameStartIndex = 0;
	}

	preIndex = currentHeapIndex;
	device->CopyDescriptorsSimple(numDescriptors, gpuHeap.handleCPU(currentHeapIndex), descriptorHeap.handleCPU(offset), gpuHeapType);
	currentHeapIndex += numDescriptors;
	return preIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE FrameManager::GetCurrentCPUHandle()
{
	return gpuHeap.handleCPU(currentHeapIndex);
}

D3D12_GPU_DESCRIPTOR_HANDLE FrameManager::GetCurrentGPUHandle()
{
	return gpuHeap.handleGPU(currentHeapIndex);
}

D3D12_GPU_DESCRIPTOR_HANDLE FrameManager::GetPreviousGPUHandle(uint32_t offset)
{
	return gpuHeap.handleGPU(preIndex + offset);
}

D3D12_GPU_DESCRIPTOR_HANDLE FrameManager::GetGPUHandle(uint32_t baseIndex, uint32_t offset)
{
	return gpuHeap.handleGPU(baseIndex + offset);
}

ID3D12DescriptorHeap* FrameManager::GetDescriptorHeap()
{
	return gpuHeap.pDescriptorHeap.Get();
}


FrameManager::~FrameManager()
{
}
