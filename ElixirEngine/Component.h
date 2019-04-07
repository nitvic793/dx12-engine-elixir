#pragma once
#include <vector>
#include "SceneCommon.h"

namespace Elixir
{
	template<typename T>
	class Component
	{
		std::vector<EntityID> entities;
		std::vector<T> component;
	public:
		Component();
		~Component();
	};
}


