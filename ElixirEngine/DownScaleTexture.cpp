#include "stdafx.h"
#include "DownScaleTexture.h"
#include "Core/DeferredRenderer.h"

DownScaleTexture::DownScaleTexture(ComputeCore* core)
{
	computeCore = core;
	downScaleCS = std::unique_ptr<ComputeProcess>(new ComputeProcess(core, L"DownScaleCS.cso"));
}

Texture * DownScaleTexture::Apply(ID3D12GraphicsCommandList * clist, Texture * input, TexturePool * pool)
{
	auto width = computeCore->GetRenderer()->GetWidth() / 4;
	auto height = computeCore->GetRenderer()->GetHeight() / 4; //Downscaling by 4x4 (1/16)
	auto tex = pool->GetNext();
	downScaleCS->SetShader(clist);
	downScaleCS->SetTextureSRV(clist, input);
	downScaleCS->SetTextureUAV(clist, tex.UAV);
	downScaleCS->Dispatch(clist, width, height);
	return tex.SRV;
}


DownScaleTexture::~DownScaleTexture()
{
}
