#include "stdafx.h"
#include "SystemManager.h"


Elixir::SystemManager::SystemManager(EntityManager * entityMgr) :
	entityManager(entityMgr)
{
}

Elixir::SystemManager::~SystemManager()
{
	for (auto system : systems)
	{
		delete system;
	}
}

void Elixir::SystemManager::Init()
{
	for (auto system : systems)
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
}

void Elixir::SystemManager::Shutdown()
{
	for (auto system : systems)
	{
		system->Shutdown();
	}
}
