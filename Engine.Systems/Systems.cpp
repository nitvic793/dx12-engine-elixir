#include "stdafx.h"
#include "Systems.h"
#include "../ElixirEngine/System.h"
#include "SampleSystem.h"

class Sys : public Elixir::ISystem
{
	// Inherited via ISystem
	virtual void Update(float deltaTime) override
	{
		entity->SetPosition(0, DirectX::XMFLOAT3(0, 0, 0));
	}
};

void LoadSystems(std::vector<Elixir::ISystem*>& systems)
{
	systems.push_back(new SampleSystem());
}

void UnloadSystems(std::vector<Elixir::ISystem*>& systems)
{
	for (auto system : systems)
	{
		delete system;
	}

	systems.clear();
}
