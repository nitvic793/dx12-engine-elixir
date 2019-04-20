#pragma once
#include "Core/ConstantBuffer.h"
#include "Serializable.h"

GameComponent(AnimationComponent)
	int	CurrentAnimationIndex;
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(CEREAL_NVP(CurrentAnimationIndex));
	}
EndComponent()

GameComponent(AnimationBufferComponent)
	PerArmatureConstantBuffer ConstantBuffer;
	template<class Archive>
	void serialize(Archive& archive)
	{
		//archive(); Don't serialize constant buffer 
	}
EndComponent()