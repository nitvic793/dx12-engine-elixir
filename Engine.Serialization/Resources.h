#pragma once

#include "Mesh.h"
#include "Material.h"
// type support
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>

// for doing the actual serialization
#include <cereal/archives/json.hpp>
#include <iostream>

struct TextureType
{
	std::string TextureID;
	std::string TexturePath;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(
			CEREAL_NVP(TextureID),
			CEREAL_NVP(TexturePath)
		);
	}
};

struct Resources
{
	std::vector<MaterialType>	Materials;
	std::vector<MeshType>		Meshes;
	std::vector<TextureType>	Textures;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(
			CEREAL_NVP(Materials),
			CEREAL_NVP(Meshes),
			CEREAL_NVP(Textures)
		);
	}
};