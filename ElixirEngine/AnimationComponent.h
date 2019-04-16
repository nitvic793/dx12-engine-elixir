#pragma once
#include "Core/ConstantBuffer.h"

struct AnimationComponent
{
	int	CurrentAnimationIndex;
};

struct AnimationBufferComponent
{
	PerArmatureConstantBuffer ConstantBuffer;
};