#include "stdafx.h"
#include "ComponentFactory.h"
#include "AnimationComponent.h"

using namespace Elixir;


ComponentFactory cf;
std::unordered_map<HashID, FactoryFunction> ComponentFactory::factoryMap;
std::unordered_map<HashID, Elixir::TypeID> ComponentFactory::typeMap;

void Elixir::ComponentFactory::RegisterComponentContainer(HashID componentId, FactoryFunction function)
{
	if (factoryMap.find(componentId) == factoryMap.end())
		factoryMap.insert(std::pair<HashID, FactoryFunction>(componentId, function));
}

void Elixir::ComponentFactory::RegisterComponentTypeID(HashID componentId, Elixir::TypeID typeId)
{
	if (typeMap.find(componentId) == typeMap.end())
	{
		typeMap.insert(std::pair<HashID, Elixir::TypeID>(componentId, typeId));
	}
}

Elixir::IComponent * Elixir::ComponentFactory::Create(HashID componentId)
{
	return factoryMap[componentId]();
}

Elixir::TypeID Elixir::ComponentFactory::GetTypeID(HashID componentId)
{
	return typeMap[componentId];
}


RegisterComponent(AnimationComponent)
RegisterComponent(AnimationBufferComponent)