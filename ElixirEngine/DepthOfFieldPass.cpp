#include "stdafx.h"
#include "DepthOfFieldPass.h"


DepthOfFieldPass::DepthOfFieldPass(ComputeCore* core):
	computeCore(core)
{
	dofCS = new ComputeProcess(core, L"DepthOfFieldCS.cso");
}

Texture * DepthOfFieldPass::Apply(ID3D12GraphicsCommandList * commandList, Texture * sharpSRV, Texture * blurSRV, TexturePool * texturePool, float focusPlaneZ, float focusLength)
{
	UINT width = 1280;
	UINT height = 720;
	dofCS->SetShader(commandList);
	dofCS->SetTextureSRV(commandList, blurSRV);
	dofCS->SetTextureSRVOffset(commandList, sharpSRV);
	dofCS->SetTextureUAV(commandList, texturePool->GetUAV(2));
	dofCS->Dispatch(commandList, width, height, 1);
	return texturePool->GetSRV(2);
}


DepthOfFieldPass::~DepthOfFieldPass()
{
	delete dofCS;
}
