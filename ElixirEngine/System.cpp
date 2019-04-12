#include "stdafx.h"
#include "System.h"

using namespace Elixir;


Elixir::ISystem::ISystem(EntityManager * entityManager):
	entity(entityManager)
{
}

Elixir::ISystem::~ISystem()
{
}

void Elixir::ISystem::GetEntities(std::vector<EntityID>& outEntities)
{
	std::vector<EntityID> entities;
	for (auto component : components)
	{
		entity->GetComponentEntities(component, entities);
		std::sort(entities.begin(), entities.end());
		if (outEntities.size() == 0)
			outEntities = entities;
		else //Get only intersecting entity list
			std::set_intersection(outEntities.begin(), outEntities.end(), entities.begin(), entities.end(), outEntities.begin()); 
	}
}

void Elixir::SampleSystem::PreUpdate()
{
	GetEntities(entities);
	GetComponents(aData, entities);
	GetComponents(bData, entities);
}

void Elixir::SampleSystem::Update(float deltaTime)
{
	totalTime += deltaTime;
	auto idx = 0;
	for (auto e : entities)
	{
		auto position = entity->GetPosition(e);
		//aData[idx]->speed = 10.f;
		position.y = aData[idx]->speed * sin(totalTime);
		entity->SetPosition(e, position);
		idx++;
	}
}

void Elixir::SampleSystem::PostUpdate()
{
	entities.clear();
	aData.clear();
	bData.clear();
}
