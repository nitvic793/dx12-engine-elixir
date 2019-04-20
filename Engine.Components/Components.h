#pragma once

#include <cereal/types/complex.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <DirectXMath.h>
using namespace DirectX;
#include "../ElixirEngine/Serializable.h"

GameComponent(Components)
	float a;
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(a));
	}
EndComponent()

GameComponent(TestA)
	float speed = 10.f;
	
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(speed));
	}
EndComponent()


GameComponent(TestB)
	float yOffset = 20.f;
	
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(yOffset));
	}
EndComponent()
