#pragma once
#include <vector>
#include "SceneCommon.h"

namespace Elixir
{
	class IComponent 
	{
	};

	template<typename T>
	class Component : public IComponent
	{
	public:
		Component();
		void AddEntity(EntityID id, const T& componentData);
		~Component();

		std::vector<T> Components;
		std::vector<EntityID> Entities;
	};

	template<typename T>
	inline void Elixir::Component<T>::AddEntity(EntityID id, const T & componentData)
	{
		Entities.push_back(id);
		Components.push_back(componentData);
	}

	template<typename T>
	Component<T>::Component()
	{
	}

	template<typename T>
	Component<T>::~Component()
	{
	}

	struct TestA {};
	struct TestB {};

}


