#include "stdafx.h"
#include "SystemManager.h"


Elixir::SystemManager::SystemManager(EntityManager * entityMgr) :
	entityManager(entityMgr)
{
}

Elixir::SystemManager::~SystemManager()
{
	for (auto system : internalSystems)
	{
		delete system;
	}
}

void Elixir::SystemManager::RegisterSystems(std::vector<ISystem*>&& systems)
{
	this->systems = std::move(systems);
	for (auto system : this->systems)
	{
		system->SetEntityManager(entityManager);
	}
}

void Elixir::SystemManager::RegisterSystems()
{
	for (auto system : systems)
	{
		system->SetEntityManager(entityManager);
		system->Init();
	}
}

std::vector<Elixir::ISystem*>& Elixir::SystemManager::GetSystems()
{
	return systems;
}

void Elixir::SystemManager::Init()
{
	for (auto system : systems)
	{
		system->Init();
	}

	for (auto system : internalSystems)
	{
		system->Init();
	}
}

void Elixir::SystemManager::Update(float deltaTime)
{
	for (auto system : systems)
	{
		system->PreUpdate();
		system->Update(deltaTime);
		system->PostUpdate();
	}

	for (auto system : internalSystems)
	{
		system->PreUpdate();
		system->Update(deltaTime);
		system->PostUpdate();
	}
}

void Elixir::SystemManager::Shutdown()
{
	for (auto system : systems)
	{
		system->Shutdown();
	}

	for (auto system : internalSystems)
	{
		system->Shutdown();
	}
}
