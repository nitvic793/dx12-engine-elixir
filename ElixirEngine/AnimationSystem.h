#pragma once
#include "System.h"
#include "AnimationComponent.h"

class AnimationSystem : public Elixir::ISystem
{
	std::vector<Elixir::EntityID> entities;
	AnimationComponent* animComponents;
	size_t componentCount;
public:
	AnimationSystem();
	~AnimationSystem();

	virtual void Init() override;
	virtual void PreUpdate() override;
	virtual void Update(float deltaTime) override;
	virtual void PostUpdate() override;
};

