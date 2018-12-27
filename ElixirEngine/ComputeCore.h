#pragma once

#include "stdafx.h"

class DeferredRenderer;

class ComputeCore
{
	ID3D12Device*			device;
	ID3D12RootSignature*	rootSignature;
	DeferredRenderer*		renderer;
	void					CreateRootSignature();
public:
	ComputeCore(ID3D12Device* device, DeferredRenderer* renderer);
	ID3D12RootSignature*	GetRootSignature();
	ID3D12Device*			GetDevice();
	DeferredRenderer*		GetRenderer();
	~ComputeCore();
};

