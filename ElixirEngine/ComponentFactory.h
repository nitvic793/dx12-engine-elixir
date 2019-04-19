#pragma once
#include "../Engine.Serialization/StringHash.h"
#include "Component.h"

namespace Elixir
{

	typedef std::function<Elixir::IComponent*()> FactoryFunction;
	class ComponentFactory
	{
		static std::unordered_map<HashID, FactoryFunction> factoryMap;
	public:
		static void RegisterComponentContainer(HashID componentId, FactoryFunction function);
		static Elixir::IComponent* Create(HashID componentId);
	};
}

