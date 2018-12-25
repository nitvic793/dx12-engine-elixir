#pragma once
#include <comdef.h>
#include <wrl/client.h>


class ConstantBufferWrapper
{
	ID3D12Resource* constantBuffer;
	int bufferSize;
	char* vAddressBegin;
public:
	ConstantBufferWrapper()
	{
		constantBuffer = nullptr;
	};
	void Initialize(ID3D12Resource* buffer, const int& bSize)
	{
		bufferSize = bSize;
		constantBuffer = buffer;
		CD3DX12_RANGE readRange(0, 0);
		buffer->Map(0, &readRange, reinterpret_cast<void**>(&vAddressBegin));
	}
	void CopyData(void* data, int size, int index)
	{
		char* ptr = reinterpret_cast<char*>(vAddressBegin) + bufferSize * index;
		memcpy(ptr, data, size);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetIndex(uint32_t index)
	{
		char* ptr = reinterpret_cast<char*>(vAddressBegin) + bufferSize * index;
		return (D3D12_GPU_VIRTUAL_ADDRESS)ptr;
	}

	~ConstantBufferWrapper()
	{
		/*if (constantBuffer)
			constantBuffer->Unmap(0, nullptr);*/
	}
};

class CDescriptorHeapWrapper
{
public:
	CDescriptorHeapWrapper() { memset(this, 0, sizeof(*this)); }

	HRESULT Create(
		ID3D12Device* pDevice,
		D3D12_DESCRIPTOR_HEAP_TYPE Type,
		UINT NumDescriptors,
		bool bShaderVisible = false)
	{
		HeapDesc.Type = Type;
		HeapDesc.NumDescriptors = NumDescriptors;
		HeapDesc.Flags = (bShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : (D3D12_DESCRIPTOR_HEAP_FLAGS)0);

		HRESULT hr = pDevice->CreateDescriptorHeap(&HeapDesc,
			__uuidof(ID3D12DescriptorHeap),
			(void**)&pDescriptorHeap);
		if (FAILED(hr)) return hr;

		hCPUHeapStart = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (bShaderVisible)
		{
			hGPUHeapStart = pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}
		else
		{
			hGPUHeapStart.ptr = 0;
		}
		HandleIncrementSize = pDevice->GetDescriptorHandleIncrementSize(HeapDesc.Type);
		return hr;
	}
	operator ID3D12DescriptorHeap*() { return pDescriptorHeap.Get(); }

	//size_t MakeOffsetted(size_t ptr, UINT index)
	//{
	//	size_t offsetted;
	//	offsetted = ptr + static_cast<size_t>(index * HandleIncrementSize);
	//	return offsetted;
	//}

	UINT64 MakeOffsetted(UINT64 ptr, UINT index)
	{
		UINT64 offsetted;
		offsetted = ptr + static_cast<UINT64>(index * HandleIncrementSize);
		return offsetted;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU(UINT index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = MakeOffsetted(hCPUHeapStart.ptr, index);
		return handle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU(UINT index)
	{
		assert(HeapDesc.Flags&D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		D3D12_GPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = MakeOffsetted(hGPUHeapStart.ptr, index);
		return handle;
	}
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE hCPUHeapStart;
	D3D12_GPU_DESCRIPTOR_HANDLE hGPUHeapStart;
	UINT HandleIncrementSize;
};