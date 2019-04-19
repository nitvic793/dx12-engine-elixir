#pragma once
#include <vector>
#include <unordered_map>
#include "SceneCommon.h"

namespace Elixir
{
	class IComponent 
	{
	public:
		virtual void GetEntities(std::vector<EntityID>& outEntities) = 0;
		virtual void Serialize(cereal::JSONOutputArchive& archive) {};
		virtual void Deserialize(cereal::JSONInputArchive& archive) {};
		virtual size_t GetHash() = 0;
		virtual void AddEntity(EntityID entity) = 0;
		virtual ~IComponent() {};
	};

	template<typename T>
	class Component : public IComponent
	{
		std::vector<EntityID> removeList;
	public:
		Component();
		void AddEntity(EntityID id, const T& componentData);
		void GetData(EntityID* entities, size_t count, T*& outComponentData);
		T&	 GetData(EntityID id);
		void RemoveEntity(EntityID id);
		~Component();

		std::vector<T> Components;
		std::vector<EntityID> Entities;
		std::unordered_map<EntityID, size_t> EntityComponentMap;

		// Inherited via IComponent
		virtual void GetEntities(std::vector<EntityID>& outEntities) override;
		
		virtual void Serialize(cereal::JSONOutputArchive& archive) 
		{
		};

		virtual void Deserialize(cereal::JSONInputArchive& archive) 
		{
		};

		template<class Archive>
		void serialize(Archive& archive)
		{
			//TODO
			//Serialize Per Component Container
			//Use Registered order consistency with virtual serialize/deserialize to load components from file

			archive(
				CEREAL_NVP(Entities),
				CEREAL_NVP(Components)
			);
		}

		virtual size_t GetHash() override
		{
			return typeid(T).hash_code();
		}

		virtual void AddEntity(EntityID entity) override
		{
			AddEntity(entity, T());
		}
	};

	template<typename T>
	inline void Elixir::Component<T>::AddEntity(EntityID id, const T & componentData)
	{
		auto cId = Components.size();
		Entities.push_back(id);
		Components.push_back(componentData);
		EntityComponentMap.insert(std::pair<EntityID, size_t>(id, cId));
	}

	template<typename T>
	inline void Component<T>::GetData(EntityID * entities, size_t count, T *& outComponentData)
	{

	}

	template<typename T>
	inline T & Component<T>::GetData(EntityID id)
	{
		auto cId = EntityComponentMap[id];
		return Components[cId];
	}

	template<typename T>
	inline void Component<T>::RemoveEntity(EntityID id)
	{
		//Swap last element and with current entity and remove from back. Swap-and-pop
		//Ensure to remove from Component vector and EntityComponentMap
	}

	template<typename T>
	Component<T>::Component()
	{
	}

	template<typename T>
	Component<T>::~Component()
	{
		EntityComponentMap.clear();
		Components.clear();
		Entities.clear();
	}

	template<typename T>
	inline void Component<T>::GetEntities(std::vector<EntityID>& outEntities)
	{
		outEntities = Entities;
	}

}

