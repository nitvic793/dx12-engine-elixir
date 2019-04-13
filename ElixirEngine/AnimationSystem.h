#pragma once
#include "System.h"
#include "AnimationManager.h"
#include "AnimationComponent.h"

class AnimationSystem : public Elixir::ISystem
{
	std::vector<Elixir::EntityID> entities;
	AnimationComponent* animComponents;
	size_t componentCount;
	AnimationManager* animManager;
	float totalTime;
public:
	AnimationSystem(AnimationManager* animManager);
	~AnimationSystem();

	virtual void Init() override;
	virtual void PreUpdate() override;
	virtual void Update(float deltaTime) override;
	virtual void PostUpdate() override;
};

