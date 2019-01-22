#pragma once

#include "Mesh.h"
#include "Material.h"

struct TextureType
{
	HashID TextureID;
	std::string TexturePath;
};

struct TextureData
{
	std::vector<TextureType> Textures;
};

struct Resources
{
	MaterialData	MaterialList;
	MeshData		MeshList;
	TextureData		TextureList;
};