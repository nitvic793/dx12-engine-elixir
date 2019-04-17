#pragma once
#include <vector>

namespace Elixir
{
	class ISystem;
}

extern "C"
{
	__declspec(dllexport) void LoadSystems(std::vector<Elixir::ISystem*>& systems);
	__declspec(dllexport) void UnloadSystems(std::vector<Elixir::ISystem*>& systems);
}