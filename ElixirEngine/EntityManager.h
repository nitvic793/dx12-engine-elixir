#pragma once
#include "stdafx.h"
#include <unordered_map>
#include "../Engine.Serialization/StringHash.h"
#include "Scene.h"
#include "SceneCommon.h"
#include "Component.h"

namespace Elixir
{
	class EntityManager;

	struct Entity
	{
		const EntityID	EntityID;
		const NodeID	Node;
		HashID			Mesh;
		HashID			Material;
		XMFLOAT4X4		WorldTransform;
		//EntityManager*	Manager;

		//Avoid using this update
		/*inline void Update();*/
	};

	class EntityManager
	{
		Scene* scene;
		std::unordered_map<std::string, EntityID> entityNameIndexMap;
		std::unordered_map<TypeID, IComponent*> components;

		std::vector<NodeID> entities;
		std::vector<HashID> meshes;
		std::vector<HashID> materials;

		std::vector<EntityID> removeList; //Entities to be removed
	public:
		EntityManager(Scene* scene);
		EntityID		CreateEntity(std::string name, const Transform& transform = DefaultTransform);
		EntityID		CreateEntity(std::string name, HashID mesh = 0u, HashID material = 0u, const Transform& transform = DefaultTransform);
		EntityID		CreateEntity(EntityID parentId, std::string name, HashID mesh = 0u, HashID material = 0u, const Transform& transform = DefaultTransform);
		//void			Remove(EntityID entity); //Remove given entity
		//void			ExecutePurge(); //Perform all remove operations at once.

		template<typename T>
		void			RegisterComponent();

		template<typename T>
		void			RegisterEntity(EntityID entity, const T& componentData = T());

		template<typename T>
		void			GetComponentEntities(std::vector<EntityID> &outEntities);

		template<typename... Args>
		void			GetMultiComponentEntities(std::vector<EntityID> &outEntities);

		//Setters
		void			SetMesh(EntityID entity, HashID mesh);
		void			SetMaterial(EntityID entity, HashID material);
		void			SetPosition(EntityID entity, const XMFLOAT3& position);
		void			SetRotation(EntityID entity, const XMFLOAT3& rotation);
		void			SetScale(EntityID entity, const XMFLOAT3& scale);
		void			SetTransform(EntityID entity, const Transform& transform);

		//Getters
		inline const XMFLOAT3&		GetPosition(EntityID entity);
		inline const XMFLOAT3&		GetRotation(EntityID entity);
		inline const XMFLOAT3&		GetScale(EntityID entity);
		const XMFLOAT4X4&			GetTransformMatrix(EntityID entity);
		inline EntityID				GetEntityID(std::string entityName) const;

		Entity			GetEntity(EntityID entity);
		void			GetEntities(std::vector<Entity>& outEntityList);
		void			UpdateEntity(const Entity& entity);

		inline size_t	Count() const { return entities.size(); };
		~EntityManager();
	};


	template<typename T>
	inline void EntityManager::RegisterComponent()
	{
		auto typeHash = typeid(T).hash_code();
		auto component = new Component<T>();
		components.insert(std::pair<TypeID, IComponent*>(typeHash, (IComponent*)component));
	}

	template<typename T>
	inline void EntityManager::RegisterEntity(EntityID entity, const T & componentData)
	{
		auto typeHash = typeid(T).hash_code();
		Component<T>* component = (Component<T>*)components[typeHash];
		component->AddEntity(entity, componentData);
	}

	template<typename T>
	inline void EntityManager::GetComponentEntities(std::vector<EntityID>& outEntities)
	{
		auto typeHash = typeid(T).hash_code();
		Component<T> component = (Component<T>)components[typeHash];
		outEntities = component.Entities;
	}

	template<typename... Args>
	inline void EntityManager::GetMultiComponentEntities(std::vector<EntityID>& outEntities)
	{
		auto list = { (components.find(typeid(Args).hash_code()))... };
		for (auto c : list)
		{
				
		}
	}

}


