#include "stdafx.h"
#include "ComponentFactory.h"
#include "AnimationComponent.h"

using namespace Elixir;


ComponentFactory cf;
std::unordered_map<HashID, FactoryFunction> ComponentFactory::factoryMap;

void Elixir::ComponentFactory::RegisterComponentContainer(HashID componentId, FactoryFunction function)
{
	if (factoryMap.find(componentId) == factoryMap.end())
		factoryMap.insert(std::pair<HashID, FactoryFunction>(componentId, function));
}

Elixir::IComponent * Elixir::ComponentFactory::Create(HashID componentId)
{
	return factoryMap[componentId]();
}


RegisterComponent(AnimationComponent)
RegisterComponent(AnimationBufferComponent)