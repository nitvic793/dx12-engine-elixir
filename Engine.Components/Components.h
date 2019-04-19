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
	Components();
	~Components();

	Component(Components)
};

struct TestA
{
	float speed = 10.f;
	Component(TestA)
};

struct TestB
{
	float yOffset = 20.f;
	Component(TestB)
};