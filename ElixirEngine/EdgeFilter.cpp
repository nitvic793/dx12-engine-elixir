#include "stdafx.h"
#include "EdgeFilter.h"
#include "Core/DeferredRenderer.h"

EdgeFilter::EdgeFilter(ComputeCore* core)
{
	computeCore = core;
	edgeDetectionCS = std::unique_ptr<ComputeProcess>(new ComputeProcess(core, L"EdgeDetectionCS.cso"));
}

Texture * EdgeFilter::Apply(ID3D12GraphicsCommandList * command, Texture * depthBuffer, Texture * pixels, TexturePool* pool)
{
	UINT width = computeCore->GetRenderer()->GetWidth();
	UINT height = computeCore->GetRenderer()->GetHeight();
	auto texResource = pool->GetNext();
	auto outSRV = texResource.SRV;
	auto outUAV = texResource.UAV;
	edgeDetectionCS->SetShader(command);
	edgeDetectionCS->SetTextureSRV(command, depthBuffer);
	edgeDetectionCS->SetTextureSRVOffset(command, pixels);
	edgeDetectionCS->SetTextureUAV(command, outUAV);
	edgeDetectionCS->Dispatch(command, width, height, 1);
	return outSRV;
}


EdgeFilter::~EdgeFilter()
{
}
