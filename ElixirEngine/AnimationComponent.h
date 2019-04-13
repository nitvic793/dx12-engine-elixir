#pragma once
#include "Core/ConstantBuffer.h"

struct AnimationComponent
{
	int	CurrentAnimationIndex;
	PerArmatureConstantBuffer ConstantBuffer;
};