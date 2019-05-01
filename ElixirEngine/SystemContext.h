#pragma once

class Camera;
class Game;
class ResourceManager;
class SystemResourceManager;
class AnimationManager;

namespace Elixir
{
	class EntityManager;
	class InputManager;

	struct SystemContext
	{
		Camera*					MainCamera;
		Game*					GameInstance;
		ResourceManager*		ResourceManager;
		EntityManager*			EntityManager;
		SystemResourceManager*	SystemResourceManager;
		AnimationManager*		AnimationManager;
		InputManager*			Input;
	};
}