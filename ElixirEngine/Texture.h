#pragma once
#include "stdafx.h"
#include "DirectXHelper.h"

enum TextureType
{
	TextureTypeSRV,
	TextureTypeUAV
};

class Texture
{
	ID3D12Resource* resource;
	CDescriptorHeapWrapper* heapDescriptor;
	TextureType textureType;
	uint32_t heapIndex;
public:
	void CreateTexture(ID3D12Resource* resource, TextureType textureType)
	{

	}
	Texture();
	~Texture();
};

