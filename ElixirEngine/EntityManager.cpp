#include "stdafx.h"
#include "EntityManager.h"

using namespace Elixir;

EntityManager::EntityManager(Scene* scene) :
	scene(scene)
{
}

EntityID Elixir::EntityManager::CreateEntity(std::string name, const Transform & transform)
{
	return CreateEntity(-1, name, 0, 0, transform);
}

EntityID Elixir::EntityManager::CreateEntity(std::string name, HashID mesh, HashID material, const Transform & transform)
{
	return CreateEntity(-1, name, mesh, material, transform);
}

EntityID Elixir::EntityManager::CreateEntity(EntityID parentId, std::string name, HashID mesh, HashID material, const Transform & transform)
{
	auto parentNode = parentId == -1 ? 0 : entities[parentId]; //If parent is root, return 0
	auto nodeId = scene->CreateNode(parentNode, transform);
	EntityID entityId = (EntityID)entities.size();
	entityNameIndexMap.insert(std::pair<std::string, EntityID>(name, entityId));
	entities.push_back(nodeId);
	meshes.push_back(mesh);
	materials.push_back(material);
	return entityId;
}

void Elixir::EntityManager::RegisterComponent(const char* componentName)
{
	HashID stringHash = StringID(componentName);
	auto typeId = ComponentFactory::GetTypeID(stringHash);
	if (components.find(typeId) == components.end())
	{
		auto component = ComponentFactory::Create(stringHash);
		components.insert(std::pair<TypeID, IComponent*>(typeId, component));
	}
}

void Elixir::EntityManager::AddComponent(EntityID entity, const char * componentName)
{
	auto typeId = ComponentFactory::GetTypeID(StringID(componentName));
	if (components.find(typeId) == components.end())
	{
		RegisterComponent(componentName);
	}
	auto component = components[typeId];
	component->AddEntity(entity);
}

void Elixir::EntityManager::GetComponentEntities(TypeID componentId, std::vector<EntityID>& outEntities)
{
	components[componentId]->GetEntities(outEntities);
}


Entity Elixir::EntityManager::GetEntity(EntityID entity)
{
	auto node = entities[entity];
	return Entity{ entity, node, meshes[entity], materials[entity], scene->GetTransformMatrix(node) };// , this};
}

void Elixir::EntityManager::GetEntities(std::vector<Entity>& outEntityList)
{
	outEntityList.clear();
	for (EntityID i = 0; i < (EntityID)entities.size(); ++i)
	{
		auto entity = GetEntity(i);
		outEntityList.push_back(entity);
	}
}

void Elixir::EntityManager::UpdateEntity(const Entity & entity)
{
	SetMesh(entity.EntityID, entity.Mesh);
	SetMaterial(entity.EntityID, entity.Material);
}

EntityManager::~EntityManager()
{
	for (auto component : components)
	{
		delete component.second;
	}
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

const XMFLOAT3 & Elixir::EntityManager::GetPosition(EntityID entity)
{
	auto node = entities[entity];
	return scene->GetTranslation(node);
}

const XMFLOAT3 & Elixir::EntityManager::GetRotation(EntityID entity)
{
	auto node = entities[entity];
	return scene->GetRotation(node);
}

const XMFLOAT3 & Elixir::EntityManager::GetScale(EntityID entity)
{
	auto node = entities[entity];
	return scene->GetScale(node);
}

const XMFLOAT4X4 & Elixir::EntityManager::GetTransformMatrix(EntityID entity)
{
	auto node = entities[entity];
	return scene->GetTransformMatrix(node);
}

EntityID Elixir::EntityManager::GetEntityID(std::string entityName)
{
	return entityNameIndexMap[entityName];
}

//inline void Elixir::Entity::Update()
//{
//	Manager->UpdateEntity(*this);
//}
