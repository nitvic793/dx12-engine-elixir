#include "stdafx.h"
#include "DepthOfFieldPass.h"


DepthOfFieldPass::DepthOfFieldPass(ComputeCore* core):
	computeCore(core)
{
	dofCS = new ComputeProcess(core, L"DepthOfFieldCS.cso");
}

Texture * DepthOfFieldPass::Apply(ID3D12GraphicsCommandList * commandList, Texture * sharpSRV, Texture * blurSRV, TexturePool * texturePool, float focusPlaneZ, float scale)
{
	UINT width = 1280;
	UINT height = 720;
	dofCS->SetShader(commandList);
	dofCS->SetConstants(commandList, &focusPlaneZ, 1, 0);
	dofCS->SetConstants(commandList, &scale, 1, 1);
	dofCS->SetTextureSRV(commandList, blurSRV);
	dofCS->SetTextureSRVOffset(commandList, sharpSRV);
	dofCS->SetTextureUAV(commandList, texturePool->GetUAV(1));
	dofCS->Dispatch(commandList, width, height, 1);
	return texturePool->GetSRV(1);
}


DepthOfFieldPass::~DepthOfFieldPass()
{
	delete dofCS;
}
