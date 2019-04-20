#include "stdafx.h"
#include "ComponentSerDe.h"
#include "ComponentFactory.h"
#include "Component.h"

using namespace Elixir;

void Elixir::ComponentSerDe::Save(std::unordered_map<TypeID, IComponent*>& components, std::string filename)
{
	std::ofstream os(filename.c_str());
	cereal::JSONOutputArchive archive(os);
	
	for (auto& component : components)
	{
		component.second->Serialize(archive);
	}
}

void Elixir::ComponentSerDe::Load(std::unordered_map<TypeID, IComponent*>& components, std::string filename)
{
	std::ifstream is(filename.c_str());
	cereal::JSONInputArchive archive(is);
	auto& typeMap = ComponentFactory::GetTypeMap();
	for (auto type : typeMap)
	{
		if (components.find(type.second) == components.end())
		{
			auto component = ComponentFactory::Create(type.first);
			component->Deserialize(archive);
			components.insert(std::pair<TypeID, IComponent*>(type.second, component));
		}
		else 
		{
			auto component = components[type.second];
			component->Deserialize(archive);
		}
	}
}

ComponentSerDe::ComponentSerDe()
{
}


ComponentSerDe::~ComponentSerDe()
{
}
