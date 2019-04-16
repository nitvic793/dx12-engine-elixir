#pragma once

#include "Texture.h"

class TextureManager
{
	std::vector<ID3D12Resource*> resources;
	std::vector<Texture*> uavs;
	std::vector<Texture*> srvs;
public:
	TextureManager();
	~TextureManager();
};

