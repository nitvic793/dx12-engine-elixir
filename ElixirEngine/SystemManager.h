#pragma once
#include "System.h"

namespace Elixir
{
	class SystemManager
	{
		EntityManager* entityManager;
		std::vector<ISystem*> systems;
	public:
		SystemManager(EntityManager* entityMgr);
		~SystemManager();

		template<typename SysType>
		void RegisterSystem();

		void Init();
		void Update(float deltaTime);
		void Shutdown();
	};

	template<typename SysType>
	inline void SystemManager::RegisterSystem()
	{
		static_assert(std::is_base_of<ISystem, SysType>::value &&
			!std::is_same<SysType, ISystem>::value,
			"ISystem must be a base class of SysType");

		ISystem* system = (ISystem*)new SysType();
		system->SetEntityManager(entityManager);
		systems.push_back(system);
	}
}


