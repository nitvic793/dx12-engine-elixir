#pragma once
#include "stdafx.h"
#include "EntityManager.h"


namespace Elixir
{
	class ISystem
	{
	protected:
		EntityManager* entity;
		std::vector<TypeID> components;

		template<typename T>
		void			GetEntities(std::vector<EntityID>& outEntities);
		void			GetEntities(std::vector<EntityID>& outEntities);
		template<typename T>
		void			GetComponents(std::vector<T*>& outComponents, const std::vector<EntityID>& entities);
	public:
		ISystem(EntityManager* entityManager);
		ISystem() { entity = nullptr; };

		void SetEntityManager(EntityManager* manager) { entity = manager; }
		template<typename T>
		void			RegisterComponent();
		virtual void	Init() {};
		virtual void	PreUpdate() {};
		virtual void	Update(float deltaTime) = 0;
		virtual void	PostUpdate() {};
		virtual void	Shutdown() {};
		virtual ~ISystem() {};
	};

	template<typename T>
	inline void ISystem::RegisterComponent()
	{
		entity->RegisterComponent<T>();
		components.push_back(typeid(T).hash_code());
	}

	template<typename T>
	inline void ISystem::GetEntities(std::vector<EntityID>& outEntities)
	{
		outEntities.clear();
		entity->GetComponentEntities<T>(outEntities);
	}

	template<typename T>
	inline void ISystem::GetComponents(std::vector<T*>& outComponents, const std::vector<EntityID>& entities)
	{
		for (auto e : entities)
		{
			T* component = &entity->GetComponent<T>(e);
			outComponents.push_back(component);
		}
	}
}


