#include "stdafx.h"
#include "DepthOfFieldPass.h"
#include "Core/DeferredRenderer.h"

DepthOfFieldPass::DepthOfFieldPass(ComputeCore* core):
	computeCore(core)
{
	dofCS = new ComputeProcess(core, L"DepthOfFieldCS.cso");
}

Texture * DepthOfFieldPass::Apply(ID3D12GraphicsCommandList * commandList, Texture * sharpSRV, Texture * blurSRV, TexturePool * texturePool, float focusPlaneZ, float scale)
{
	auto texResource = texturePool->GetNext();
	UINT width = computeCore->GetRenderer()->GetWidth();
	UINT height = computeCore->GetRenderer()->GetHeight();
	dofCS->SetShader(commandList);
	dofCS->SetConstants(commandList, &focusPlaneZ, 1, 0);
	dofCS->SetConstants(commandList, &scale, 1, 1);
	dofCS->SetTextureSRV(commandList, blurSRV);
	dofCS->SetTextureSRVOffset(commandList, sharpSRV);
	dofCS->SetTextureUAV(commandList, texResource.UAV);
	dofCS->Dispatch(commandList, width, height, 1);
	return texResource.SRV;
}


DepthOfFieldPass::~DepthOfFieldPass()
{
	delete dofCS;
}
