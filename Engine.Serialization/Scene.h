#pragma once

#include "Resources.h"

#include <DirectXMath.h>
#include <cereal/types/complex.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

using namespace DirectX;

struct Vector3
{
	XMFLOAT3 Vector;

	operator XMFLOAT3()
	{
		return Vector;
	}

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(
			CEREAL_NVP(Vector.x),
			CEREAL_NVP(Vector.y),
			CEREAL_NVP(Vector.z)
		);
	}
};

struct EntityType
{
	Vector3		Position;
	Vector3		Scale;
	Vector3		Rotation;
	std::string MaterialID;
	std::string MeshID;
	bool		CastShadows;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(
			CEREAL_NVP(Position),
			CEREAL_NVP(Scale),
			CEREAL_NVP(Rotation),
			CEREAL_NVP(MaterialID),
			CEREAL_NVP(MeshID),
			CEREAL_NVP(CastShadows)
		);
	}
};

struct Scene
{
	std::vector<EntityType> Entities;

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(
			CEREAL_NVP(Entities)
		);
	}
};
