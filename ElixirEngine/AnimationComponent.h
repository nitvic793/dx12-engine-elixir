#pragma once
#include "Core/ConstantBuffer.h"
#include "Serializable.h"

struct AnimationComponent
{
	int	CurrentAnimationIndex;
	GameComponent(AnimationComponent)

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(CurrentAnimationIndex));
	}
};

struct AnimationBufferComponent
{
	PerArmatureConstantBuffer ConstantBuffer;

	GameComponent(AnimationBufferComponent)
	template<class Archive>
	void serialize(Archive& archive)
	{
		//archive();
	}
};