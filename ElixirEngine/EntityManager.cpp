#include "stdafx.h"
#include "EntityManager.h"

using namespace Elixir;

EntityManager::EntityManager(Scene* scene):
	scene(scene)
{
}

EntityID Elixir::EntityManager::CreateEntity(std::string name, HashID mesh, HashID material, const Transform & transform)
{
	return CreateEntity(0, name, mesh, material, transform);
}

EntityID Elixir::EntityManager::CreateEntity(EntityID parentId, std::string name, HashID mesh, HashID material, const Transform & transform)
{
	auto parentNode = entities[parentId];
	auto nodeId = scene->CreateNode(parentNode, transform);
	EntityID entityId = (EntityID)entities.size();
	entityNameIndexMap.insert(std::pair<std::string, EntityID>(name, entityId));
	entities.push_back(nodeId);
	meshes.push_back(mesh);
	materials.push_back(material);
	return entityId;
}


EntityManager::~EntityManager()
{
}
