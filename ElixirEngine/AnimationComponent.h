#pragma once
#include "Core/ConstantBuffer.h"

struct AnimationComponent
{
	PerArmatureConstantBuffer	ConstantBuffer;
	int							CurrentAnimationIndex;
};