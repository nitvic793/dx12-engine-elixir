#pragma once
#include "System.h"

namespace Elixir
{
	class SystemManager
	{
		EntityManager* entityManager;
		std::vector<ISystem*> systems;
		std::vector<ISystem*> internalSystems;
	public:
		SystemManager(EntityManager* entityMgr);
		~SystemManager();

		template<typename SysType, typename ...Args>
		void RegisterSystem(Args&& ...args);

		void RegisterSystems(std::vector<ISystem*>&& systems);
		void RegisterSystems();
		std::vector<ISystem*>& GetSystems();

		void Init();
		void Update(float deltaTime);
		void Shutdown();
	};

	template<typename SysType, typename ...Args>
	inline void SystemManager::RegisterSystem(Args&& ...args)
	{
		static_assert(std::is_base_of<ISystem, SysType>::value &&
			!std::is_same<SysType, ISystem>::value,
			"ISystem must be a base class of SysType");

		ISystem* system = (ISystem*)new SysType(args...);
		system->SetEntityManager(entityManager);
		internalSystems.push_back(system);
	}
}


