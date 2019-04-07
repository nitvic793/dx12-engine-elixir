#pragma once
#include "stdafx.h"
#include <unordered_map>
#include "../Engine.Serialization/StringHash.h"
#include "Scene.h"

namespace Elixir
{
	typedef int EntityID;

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

		//Getters
		const XMFLOAT3& GetPosition(EntityID entity);
		const XMFLOAT3& GetRotation(EntityID entity);
		const XMFLOAT3& GetScale(EntityID entity);
		EntityID		GetEntityHandle(std::string entityName);
		~EntityManager();
	};
}


