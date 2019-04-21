#pragma once

class Game;
class ResourceManager;
class SystemResourceManager;
class AnimationManager;

namespace Elixir
{
	class EntityManager;

	struct SystemContext
	{
		Game*					GameInstance;
		ResourceManager*		ResourceManager;
		EntityManager*			EntityManager;
		SystemResourceManager*	SystemResourceManager;
		AnimationManager*		AnimationManager;
	};
}