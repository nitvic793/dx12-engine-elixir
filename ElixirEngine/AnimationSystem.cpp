#include "stdafx.h"
#include "AnimationSystem.h"
#include "AnimationComponent.h"

AnimationSystem::AnimationSystem()
{
}


AnimationSystem::~AnimationSystem()
{
}

void AnimationSystem::Init()
{
	RegisterComponent<AnimationComponent>();
}

void AnimationSystem::PreUpdate()
{
	GetEntities<AnimationComponent>(entities);
	animComponents = entity->GetComponents<AnimationComponent>(componentCount);
}

void AnimationSystem::Update(float deltaTime)
{
	for (size_t i = 0; i < entities.size(); ++i)
	{
		auto component = animComponents[i];
	}
}

void AnimationSystem::PostUpdate()
{
}
