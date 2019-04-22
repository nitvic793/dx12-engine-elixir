#pragma once
#include "ComponentFactory.h"
#include <cereal/types/complex.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>

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

//! Lets the compiler know that this struct is a in-game component
#define GameComponent(name) \
	struct name : public Elixir::IComponentData { \
	static Elixir::Serializable<name> Reflectable_ ## name; \
	static const char* GetName() \
	{ \
		return #name ; \
    } \
	virtual IComponentData* Clone() override \
	{ \
		name* val = new name(); \
		*val = *this; \
		return val; \
	}

#define EndComponent(name) \
	}; \
	CEREAL_REGISTER_TYPE(name) \
	CEREAL_REGISTER_POLYMORPHIC_RELATION(Elixir::IComponentData, name)

#define RegisterComponent(name)\
	Elixir::Serializable<name> name ## ::Reflectable_ ## name = Elixir::Serializable<name>(#name); 


#define Construct(type) \
	static const char* GetName() \
	{ \
		return Reflectable_ ## type.Name; \
    }
}