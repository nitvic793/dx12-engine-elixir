#pragma once
#include "stdafx.h"
#include "Core/ShaderManager.h"

class ComputeProcess
{
	ID3D12PipelineState*	pipelineStateObject;
	ID3D12Device*			device;
	ID3D12RootSignature*	rootSignature;
	std::wstring			shaderFileName;

	void CreatePSO();
	void CreateRootSignature();
public:
	ComputeProcess(ID3D12Device* device, std::wstring filename);
	~ComputeProcess();

	void Dispatch(ID3D12GraphicsCommandList* commandList);
};

