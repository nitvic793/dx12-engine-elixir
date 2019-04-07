#include "stdafx.h"
#include "EntityManager.h"

using namespace Elixir;

EntityManager::EntityManager(Scene* scene) :
	scene(scene)
{
}

EntityID Elixir::EntityManager::CreateEntity(std::string name, const Transform & transform)
{
	return CreateEntity(0, name, 0, 0, transform);
}

EntityID Elixir::EntityManager::CreateEntity(std::string name, HashID mesh, HashID material, const Transform & transform)
{
	return CreateEntity(0, name, mesh, material, transform);
}

EntityID Elixir::EntityManager::CreateEntity(EntityID parentId, std::string name, HashID mesh, HashID material, const Transform & transform)
{
	auto parentNode = parentId == 0 ? 0 : entities[parentId]; //If parent is root, return 0
	auto nodeId = scene->CreateNode(parentNode, transform);
	EntityID entityId = (EntityID)entities.size();
	entityNameIndexMap.insert(std::pair<std::string, EntityID>(name, entityId));
	entities.push_back(nodeId);
	meshes.push_back(mesh);
	materials.push_back(material);
	return entityId;
}


Entity Elixir::EntityManager::GetEntity(EntityID entity)
{
	auto node = entities[entity];
	return { entity, node, meshes[entity], materials[entity], scene->GetTransform(node), this };
}

void Elixir::EntityManager::UpdateEntity(const Entity & entity)
{
	SetTransform(entity.EntityID, entity.Transform);
	SetMesh(entity.EntityID, entity.Mesh);
	SetMaterial(entity.EntityID, entity.Material);
}

EntityManager::~EntityManager()
{
}

void Elixir::EntityManager::SetMesh(EntityID entity, HashID mesh)
{
	meshes[entity] = mesh;
}

void Elixir::EntityManager::SetMaterial(EntityID entity, HashID material)
{
	materials[entity] = material;
}

void Elixir::EntityManager::SetPosition(EntityID entity, const XMFLOAT3 & position)
{
	auto node = entities[entity];
	scene->SetTranslation(node, position);
}

void Elixir::EntityManager::SetRotation(EntityID entity, const XMFLOAT3 & rotation)
{
	auto node = entities[entity];
	scene->SetRotation(node, rotation);
}

void Elixir::EntityManager::SetScale(EntityID entity, const XMFLOAT3 & scale)
{
	auto node = entities[entity];
	scene->SetScale(node, scale);
}

void Elixir::EntityManager::SetTransform(EntityID entity, const Transform & transform)
{
	auto node = entities[entity];
	scene->SetTransform(node, transform);
}
