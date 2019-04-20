#pragma once
#include "../Engine.Serialization/StringHash.h"
#include "Component.h"

namespace Elixir
{
	typedef std::function<Elixir::IComponent*()> FactoryFunction;
	class ComponentFactory
	{
		static std::unordered_map<HashID, FactoryFunction> factoryMap;
		static std::unordered_map<HashID, Elixir::TypeID> typeMap;
	public:
		static void RegisterComponentContainer(HashID componentId, FactoryFunction function);
		static void RegisterComponentTypeID(HashID componentId, Elixir::TypeID typeId);
		static std::unordered_map<HashID, Elixir::TypeID>& GetTypeMap() { return typeMap; }
		static Elixir::IComponent* Create(HashID componentId);
		static Elixir::TypeID GetTypeID(HashID componentId);
	};
}

