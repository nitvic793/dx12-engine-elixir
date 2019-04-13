#include "stdafx.h"
#include "AnimationSystem.h"
#include "AnimationComponent.h"

AnimationSystem::AnimationSystem(AnimationManager* animManager) :
	animManager(animManager)
{
}


AnimationSystem::~AnimationSystem()
{
}

void AnimationSystem::Init()
{
	RegisterComponent<AnimationComponent>();
	totalTime = 0.f;
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
		auto component = &animComponents[i];
		auto animIndex = component->CurrentAnimationIndex;
		auto sEntity = entity->GetEntity(entities[i]);
		animManager->BoneTransform(entities[i], sEntity.Mesh, animIndex, totalTime, &component->ConstantBuffer);
	}
	totalTime += deltaTime;
}

void AnimationSystem::PostUpdate()
{
}
