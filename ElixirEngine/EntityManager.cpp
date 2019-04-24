#include "stdafx.h"
#include "EntityManager.h"
#include "ComponentSerDe.h"
#include "ResourceManager.h"
#include "AnimationManager.h"
#include <memory>

using namespace Elixir;

EntityManager::EntityManager(Elixir::Scene* scene) :
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
	nodeMap.insert_or_assign(nodeId, entityId);
	meshes.push_back(mesh);
	materials.push_back(material);
	parents.push_back(parentId);
	active.push_back(true);
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

void Elixir::EntityManager::AddComponent(EntityID entity, const char * componentName, IComponentData* data)
{
	auto typeId = ComponentFactory::GetTypeID(StringID(componentName));
	if (components.find(typeId) == components.end())
	{
		RegisterComponent(componentName);
	}
	auto component = components[typeId];
	component->AddEntity(entity, data);
}

void Elixir::EntityManager::GetComponentEntities(TypeID componentId, std::vector<EntityID>& outEntities)
{
	std::vector<EntityID> compEntities;
	components[componentId]->GetEntities(compEntities);
	for (auto e : compEntities)
	{
		if (active[e])
		{
			outEntities.push_back(e);
		}
	}
}


Elixir::Entity Elixir::EntityManager::GetEntity(EntityID entity)
{
	auto node = entities[entity];
	return Entity{ entity, node, meshes[entity], materials[entity], scene->GetTransformMatrix(node) };// , this};
}

void Elixir::EntityManager::GetEntities(std::vector<Entity>& outEntityList)
{
	outEntityList.clear();
	for (EntityID i = 0; i < (EntityID)entities.size(); ++i)
	{
		if (active[i])
		{
			auto entity = GetEntity(i);
			outEntityList.push_back(entity);
		}
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

Elixir::IComponentData * Elixir::EntityManager::GetComponent(const char * componentName, EntityID entity)
{
	auto type = ComponentFactory::GetTypeID(StringID(componentName));
	auto data = components[type]->GetComponentData(entity);
	return data;
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

void Elixir::EntityManager::SetActive(EntityID entity, bool enable)
{
	active[entity] = (byte)enable;
	auto node = entities[entity];
	std::vector<NodeID> children;
	scene->GetChildren(node, children);
	for (auto child : children)
	{
		auto entity = nodeMap[child];
		active[entity] = (byte)enable;
	}
}

void Elixir::EntityManager::SaveComponentsToFile(const char * filename)
{
	ComponentSerDe::Save(components, filename);
}

void Elixir::EntityManager::LoadComponentsFromFile(const char * filename)
{
	ComponentSerDe::Load(components, filename);
}

void Elixir::EntityManager::SaveToFile(const char * filename, ResourceManager* rm)
{
	std::vector<EntityInterface> outEntities;
	EntityID id = 0;
	for (NodeID entityNode : entities)
	{
		EntityInterface e;
		e.ParentID = parents[id];
		e.Mesh = rm->GetString(meshes[id]);
		e.Material = rm->GetString(materials[id]);
		e.Position.Value = GetPosition(id);
		e.Scale.Value = GetScale(id);
		e.Rotation.Value = GetRotationInDegrees(id);
		for (auto comp : components)
		{
			auto data = comp.second->GetComponentData(id);
			if (data != nullptr)
			{
				auto compRef = data->Clone();
				auto container = ComponentContainer{
					comp.second->GetComponentName(),
					std::shared_ptr<IComponentData>(compRef)
				};
				e.Components.push_back(container);
			}
		}
		outEntities.push_back(e);
		id++;
	}
	
	{
		std::ofstream os(filename);
		cereal::JSONOutputArchive archive(os);
		archive(cereal::make_nvp("Scene", outEntities));
	}
}

void ConvertDegreesToRadians(XMFLOAT3& rot)
{
	rot.x = XMConvertToRadians(rot.x);
	rot.y = XMConvertToRadians(rot.y);
	rot.z = XMConvertToRadians(rot.z);
}

void Elixir::EntityManager::LoadFromFile(const char * filename, SystemContext context)
{
	auto rm = context.ResourceManager;
	auto am = context.AnimationManager;
	std::vector<EntityInterface> outEntities;
	std::ifstream is(filename);
	cereal::JSONInputArchive archive(is);
	archive(cereal::make_nvp("Scene", outEntities));
	int index = 0;
	for (auto e : outEntities)
	{
		auto meshId = StringID(e.Mesh);
		ConvertDegreesToRadians(e.Rotation.Value);
		auto id = CreateEntity(e.ParentID, std::to_string(index), meshId, StringID(e.Material), Transform{
			e.Position.Value,
			e.Rotation.Value,
			e.Scale.Value
		});

		for (auto comp : e.Components)
		{
			auto typeId = ComponentFactory::GetTypeID(StringID(comp.ComponentName));
			AddComponent(id, comp.ComponentName.c_str(), (IComponentData*)comp.Data.get());
		}

		if (rm->GetMesh(meshId)->IsAnimated())
		{
			am->RegisterEntity(id, meshId);
		}
		index++;
	}
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

XMFLOAT3 Elixir::EntityManager::GetRotationInDegrees(EntityID entity)
{
	XMFLOAT3 rotation = GetRotation(entity);
	rotation.x = rotation.x * XM_1DIVPI * 180.f;
	rotation.y = rotation.y * XM_1DIVPI * 180.f;
	rotation.z = rotation.z * XM_1DIVPI * 180.f;
	return rotation;
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

const bool Elixir::EntityManager::IsActive(EntityID entity)
{
	return active[entity];
}

//inline void Elixir::Entity::Update()
//{
//	Manager->UpdateEntity(*this);
//}
