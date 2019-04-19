#pragma once
#include "Core/ConstantBuffer.h"
#include "Serializable.h"

struct AnimationComponent
{
	int	CurrentAnimationIndex;
	Component(AnimationComponent)
};

struct AnimationBufferComponent
{
	PerArmatureConstantBuffer ConstantBuffer;
	Component(AnimationBufferComponent)
};