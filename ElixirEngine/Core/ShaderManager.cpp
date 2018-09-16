#include "ShaderManager.h"

D3D12_SHADER_BYTECODE ShaderManager::LoadShader(std::wstring shaderCsoFile)
{
	ID3DBlob* shaderBlob;
	D3DReadFileToBlob(shaderCsoFile.c_str(), &shaderBlob);
	D3D12_SHADER_BYTECODE shaderBytecode = {};
	shaderBytecode.BytecodeLength = shaderBlob->GetBufferSize();
	shaderBytecode.pShaderBytecode = shaderBlob->GetBufferPointer();
	return shaderBytecode;
}
