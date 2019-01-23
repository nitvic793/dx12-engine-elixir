#pragma once

#include "stdafx.h"

// type support
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

struct MeshType
{
	std::string MeshID;
	std::string MeshPath;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(
			CEREAL_NVP(MeshID),
			CEREAL_NVP(MeshPath)
		);
	}
};