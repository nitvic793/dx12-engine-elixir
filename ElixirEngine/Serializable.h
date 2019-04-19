#pragma once
#include "ComponentFactory.h"

namespace Elixir
{
	template<typename T>
	class Serializable
	{
		static void RegisterType(const char* name) 
		{
			ComponentFactory::RegisterComponentContainer(
				StringID(name), 
				[&]()->Elixir::IComponent* 
				{
					return (Elixir::IComponent*)new Elixir::Component<T>();
				}
			);

			ComponentFactory::RegisterComponentTypeID(StringID(name), typeid(T).hash_code());
		};

	public:
		Serializable(const char* name) :
			Name(name)
		{
			RegisterType(name);
		}

		const char* Name;
	};

//Lets the compiler know that this struct is a in-game component
#define Component(name) \
	static Elixir::Serializable<name> Reflectable_ ## name; \
	static const char* GetName() \
	{ \
		return Reflectable_ ## name.Name; \
    }

#define RegisterComponent(name)\
	Elixir::Serializable<name> name ## ::Reflectable_ ## name = Elixir::Serializable<name>(#name); 

#define Construct(type) \
	static const char* GetName() \
	{ \
		return Reflectable_ ## type.Name; \
    }
}