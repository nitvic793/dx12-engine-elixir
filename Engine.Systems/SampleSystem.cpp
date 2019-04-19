#include "stdafx.h"
#include "SampleSystem.h"


void SampleSystem::PreUpdate()
{
	GetEntities(entities);
	GetComponents(aData, entities); 
	GetComponents(bData, entities); 
}

void SampleSystem::Update(float deltaTime)
{
	totalTime += deltaTime;
	auto idx = 0;
	for (auto e : entities)
	{ 
		auto position = entity->GetPosition(e);
		//aData[idx]->speed = 10.f;s
		position.z = aData[idx]->speed * sin(totalTime);
		entity->SetPosition(e, position);
		idx++;
	}
}

void SampleSystem::PostUpdate()
{
	entities.clear();
	aData.clear();
	bData.clear();
}

