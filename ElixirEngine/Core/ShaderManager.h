#pragma once
#include "../stdafx.h"

class ShaderManager
{
public:
	static D3D12_SHADER_BYTECODE LoadShader(std::wstring shaderCsoFile);
};