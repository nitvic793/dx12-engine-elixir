#pragma once
#include "stdafx.h"
#include "Core/ShaderManager.h"
#include "ComputeCore.h"
#include "Texture.h"

class ComputeProcess
{
	ID3D12PipelineState*	pipelineStateObject;
	ComputeCore*			computeCore;
	std::wstring			shaderFileName;

	void CreatePSO();
public:
	ComputeProcess(ComputeCore* core, std::wstring filename);
	~ComputeProcess();

	void SetShader(ID3D12GraphicsCommandList* commandList);
	void SetTextureUAV(ID3D12GraphicsCommandList* commandList, Texture* textureUAV);
	void SetTextureSRV(ID3D12GraphicsCommandList* commandList, Texture* textureSRV);
	void SetConstants(ID3D12GraphicsCommandList* commandList, void* data, UINT count, UINT offset);
	void Dispatch(ID3D12GraphicsCommandList* commandList, int x = 1, int y = 1, int z = 1);
};

