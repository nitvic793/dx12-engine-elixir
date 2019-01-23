#pragma once

#include "stdafx.h"
// type support
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>

// for doing the actual serialization
#include <cereal/archives/json.hpp>
#include <iostream>

struct MaterialType
{
	std::string MaterialID;
	std::string AlbedoPath;
	std::string NormalPath;
	std::string RoughnessPath;
	std::string MetalnessPath;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(
			CEREAL_NVP(MaterialID),
			CEREAL_NVP(AlbedoPath),
			CEREAL_NVP(NormalPath),
			CEREAL_NVP(RoughnessPath),
			CEREAL_NVP(MetalnessPath)
		);
	}
};