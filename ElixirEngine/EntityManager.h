#pragma once
#include "stdafx.h"
#include <unordered_map>
#include "../Engine.Serialization/StringHash.h"
#include "Scene.h"
#include "SceneCommon.h"

namespace Elixir
{
	class EntityManager;

	struct Entity
	{
		const EntityID	EntityID;
		const NodeID	Node;
		HashID			Mesh;
		HashID			Material;
		Transform		Transform;

		//Avoid using this update
		inline void Update() { Manager->UpdateEntity(*this); };

	protected:
		EntityManager*	Manager;
	};

	class EntityManager
	{
		Scene* scene;
		std::unordered_map<std::string, EntityID> entityNameIndexMap;
		std::vector<NodeID> entities;
		std::vector<HashID> meshes;
		std::vector<HashID> materials;

		std::vector<EntityID> removeList; //Entities to be removed
	public:
		EntityManager(Scene* scene);
		EntityID		CreateEntity(std::string name, const Transform& transform = DefaultTransform);
		EntityID		CreateEntity(std::string name, HashID mesh = 0u, HashID material = 0u, const Transform& transform = DefaultTransform);
		EntityID		CreateEntity(EntityID parentId, std::string name, HashID mesh = 0u, HashID material = 0u, const Transform& transform = DefaultTransform);
		void			Remove(EntityID entity); //Remove given entity
		void			ExecutePurge(); //Perform all remove operations at once.

		//Setters
		void			SetMesh(EntityID entity, HashID mesh);
		void			SetMaterial(EntityID entity, HashID material);
		void			SetPosition(EntityID entity, const XMFLOAT3& position);
		void			SetRotation(EntityID entity, const XMFLOAT3& rotation);
		void			SetScale(EntityID entity, const XMFLOAT3& scale);
		void			SetTransform(EntityID entity, const Transform& transform);

		//Getters
		const XMFLOAT3& GetPosition(EntityID entity);
		const XMFLOAT3& GetRotation(EntityID entity);
		const XMFLOAT3& GetScale(EntityID entity);
		EntityID		GetEntityID(std::string entityName) const;

		Entity			GetEntity(EntityID entity);
		void			UpdateEntity(const Entity& entity);

		inline size_t	Count() const { return entities.size(); };
		~EntityManager();
	};
}


