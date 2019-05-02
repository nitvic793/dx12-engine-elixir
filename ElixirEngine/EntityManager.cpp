#include "stdafx.h"
#include "EntityManager.h"
#include "ComponentSerDe.h"
#include "ResourceManager.h"
#include "AnimationManager.h"
#include <memory>

using namespace Elixir;

EntityManager* EntityManager::Instance = nullptr;

struct EntitySerialInterface
{
	Elixir::EntityID	EntityID;
	Elixir::EntityID	ParentID;
	std::string			Mesh;
	std::string			Material;
	Elixir::Vector3		Position;
	Elixir::Vector3		Rotation;
	Elixir::Vector3		Scale;
	std::vector<std::string> AttachedComponents;

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(EntityID),
			CEREAL_NVP(ParentID),
			CEREAL_NVP(Mesh),
			CEREAL_NVP(Material),
			CEREAL_NVP(Position),
			CEREAL_NVP(Rotation),
			CEREAL_NVP(Scale),
			CEREAL_NVP(AttachedComponents)
		);

		auto em = EntityManager::GetInstance();
		for (auto comp : AttachedComponents)
		{
			auto typeId = ComponentFactory::GetTypeID(StringID(comp));
			auto c = em->GetComponentContainer(comp.c_str());
			c->Serialize(archive, EntityID);
		}
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(EntityID),
			CEREAL_NVP(ParentID),
			CEREAL_NVP(Mesh),
			CEREAL_NVP(Material),
			CEREAL_NVP(Position),
			CEREAL_NVP(Rotation),
			CEREAL_NVP(Scale),
			CEREAL_NVP(AttachedComponents)
		);

		auto em = EntityManager::GetInstance();
		for (auto comp : AttachedComponents)
		{
			auto typeId = ComponentFactory::GetTypeID(StringID(comp));
			auto c = em->GetComponentContainer(comp.c_str());
			if (c == nullptr)
			{
				em->RegisterComponent(comp.c_str());
				c = em->GetComponentContainer(comp.c_str());
			}
			c->Deserialize(archive, EntityID);
		}
	}
};


EntityManager::EntityManager(Elixir::Scene* scene) :
	scene(scene)
{
	Instance = this;
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
	auto parentNode = parentId == -1 ? RootNodeID : entities[parentId]; //If parent is root, return 0
	auto nodeId = scene->CreateNode(parentNode, transform);
	EntityID entityId;
	if (freeEntityIds.size() > 0)
	{
		entityId = freeEntityIds.back();
		freeEntityIds.pop_back();
		entities[entityId] = nodeId;
		nodeMap[nodeId] = entityId;
		meshes[entityId] = mesh;
		materials[entityId] = material;
		parents[entityId] = parentId;
		active[entityId] = true;
	}
	else
	{
		entityId = (EntityID)entities.size();
		entityNameIndexMap.insert(std::pair<std::string, EntityID>(name, entityId));
		entities.push_back(nodeId);
		nodeMap.insert_or_assign(nodeId, entityId);
		meshes.push_back(mesh);
		materials.push_back(material);
		parents.push_back(parentId);
		active.push_back(true);
	}

	return entityId;
}

void Elixir::EntityManager::Remove(EntityID entity)
{
	freeEntityIds.push_back(entity);
	auto node = entities[entity];
	std::vector<NodeID> removedChildNodes;
	std::vector<EntityID> removedEntities;

	removedEntities.push_back(entity);
	scene->RemoveNode(node, removedChildNodes);

	for (auto node : removedChildNodes)
	{
		auto childEntity = nodeMap[node];
		freeEntityIds.push_back(childEntity);
		removedEntities.push_back(childEntity);
	}

	for (auto removed : removedEntities)
	{
		parents[removed] = RootNodeID;
		active[removed] = false;
		for (auto comp : components)
		{
			comp.second->RemoveEntity(removed);
		}
		//entityNameIndexMap.erase() remove from name index map
	}
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
	else if (components[typeId] == nullptr)
	{
		components[typeId] = ComponentFactory::Create(stringHash);
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

IComponent * Elixir::EntityManager::GetComponentContainer(const char * componentName)
{
	auto type = ComponentFactory::GetTypeID(StringID(componentName));
	return components[type];
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

void Elixir::EntityManager::SaveToFile(const char * filename, ResourceManager* rm)
{
	std::vector<EntitySerialInterface> outEntities;
	std::vector<IComponentData*> comps;
	EntityID id = 0;
	for (NodeID entityNode : entities)
	{
		EntitySerialInterface e;
		e.EntityID = id;
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
				e.AttachedComponents.push_back(comp.second->GetComponentName());
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

void Elixir::EntityManager::Load(const char * filename, SystemContext context)
{
	auto rm = context.ResourceManager;
	auto am = context.AnimationManager;
	std::vector<EntitySerialInterface> outEntities;
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

