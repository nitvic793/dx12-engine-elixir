#include "stdafx.h"
#include "DepthOfFieldPass.h"


DepthOfFieldPass::DepthOfFieldPass(ComputeCore* core):
	computeCore(core)
{
	dofCS = new ComputeProcess(core, L"DepthOfFieldCS.cso");
}

Texture * DepthOfFieldPass::Apply(ID3D12GraphicsCommandList * commandList, Texture * sharpSRV, Texture * blurSRV, TexturePool * texturePool, float focusPlaneZ, float focusLength)
{
	return nullptr;
}


DepthOfFieldPass::~DepthOfFieldPass()
{
	delete dofCS;
}
