#pragma once

#include <cereal/types/complex.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <DirectXMath.h>
using namespace DirectX;
#include "../ElixirEngine/Serializable.h"

struct Components
{
	float a;
	 
	GameComponent(Components)
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(a));
	}
};

struct TestA
{
	float speed = 10.f;
	GameComponent(TestA)
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(speed));
	}
};

struct TestB
{
	float yOffset = 20.f;
	GameComponent(TestB)
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(yOffset));
	}
};