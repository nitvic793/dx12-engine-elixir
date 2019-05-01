#pragma once
#include <Keyboard.h>
#include <Mouse.h>

namespace Elixir
{
	enum MouseButton
	{
		MBLeft,
		MBRight,
		MBMiddle
	};

	struct InputContext
	{
		DirectX::Keyboard*	KeyboardInstance;
		DirectX::Mouse*		MouseInstance;
	};

	struct InputState
	{
		DirectX::Keyboard::State	KeyboardState;
		DirectX::Mouse::State		MouseState;
	};

	class InputManager
	{
		InputContext				context;
		std::atomic<InputState>		state;
		InputManager() {};
	public:
		static InputManager& GetInstance()
		{
			static InputManager instance;
			return instance;
		}

		static void SetContext(InputContext inputContext)
		{
			auto& instance = GetInstance();
			instance.context = inputContext;
		}

		void Update();
		const DirectX::Keyboard::State	GetKeyboardState();
		const DirectX::Mouse::State		GetMouseState();

		bool IsKeyDown(DirectX::Keyboard::Keys key);
		bool IsKeyUp(DirectX::Keyboard::Keys key);
		bool IsMouseDown(MouseButton button);

		InputManager(InputManager const&) = delete;
		void operator=(InputManager const&) = delete;
	};

	namespace Input
	{
		bool IsKeyDown(DirectX::Keyboard::Keys key);
		bool IsKeyUp(DirectX::Keyboard::Keys key);
		bool IsMouseDown(MouseButton button);
	}
}