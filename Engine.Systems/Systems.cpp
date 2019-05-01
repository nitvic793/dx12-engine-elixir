#include "stdafx.h"
#include "Systems.h"
#include "../ElixirEngine/System.h"
#include "SampleSystem.h"
#include "RotatorSystem.h"


void LoadSystems(std::vector<Elixir::ISystem*>& systems)
{
	systems.push_back(new SampleSystem());
	systems.push_back(new RotatorSystem());
}

void UnloadSystems(std::vector<Elixir::ISystem*>& systems)
{
	for (auto system : systems)
	{
		delete system;
	}

	systems.clear();
}
