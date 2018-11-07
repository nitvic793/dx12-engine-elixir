#pragma once
#include "stdafx.h"
#include "Core/ShaderManager.h"
#include "Texture.h"

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

	void SetShader(ID3D12GraphicsCommandList* commandList);
	void SetTextureUAV(ID3D12GraphicsCommandList* commandList, Texture* textureUAV);
	void Dispatch(ID3D12GraphicsCommandList* commandList, int x = 1, int y = 1, int z = 1);
};

