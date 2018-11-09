#pragma once

#include "stdafx.h"

class ComputeCore
{
	ID3D12Device*			device;
	ID3D12RootSignature*	rootSignature;

	void CreateRootSignature();
public:
	ComputeCore(ID3D12Device* device);
	ID3D12RootSignature* GetRootSignature();
	ID3D12Device* GetDevice();
	~ComputeCore();
};

