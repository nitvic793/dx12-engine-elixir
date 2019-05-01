#include "stdafx.h"
#include "Input.h"

void Elixir::InputManager::Update()
{
	InputState newState;
	newState.KeyboardState = context.KeyboardInstance->GetState();
	newState.MouseState = context.MouseInstance->GetState();
	state = newState;
}

const DirectX::Keyboard::State Elixir::InputManager::GetKeyboardState()
{
	return ((Elixir::InputState)this->state).KeyboardState;
}

const DirectX::Mouse::State Elixir::InputManager::GetMouseState()
{
	return ((Elixir::InputState)this->state).MouseState;
}

bool Elixir::InputManager::IsKeyDown(DirectX::Keyboard::Keys key)
{
	auto kb = GetKeyboardState();
	return kb.IsKeyDown(key);
}

bool Elixir::InputManager::IsKeyUp(DirectX::Keyboard::Keys key)
{
	auto kb = GetKeyboardState();
	return kb.IsKeyUp(key);
}

bool Elixir::InputManager::IsMouseDown(MouseButton button)
{
	auto mouse = GetMouseState();
	switch (button)
	{
	case MBLeft:
		return mouse.leftButton;
	case MBRight:
		return mouse.rightButton;
	case MBMiddle:
		return mouse.middleButton;
	}

	return false;
}

bool Elixir::Input::IsKeyDown(DirectX::Keyboard::Keys key)
{
	auto kb = Elixir::InputManager::GetInstance().GetKeyboardState();
	return kb.IsKeyDown(key);
}

bool Elixir::Input::IsKeyUp(DirectX::Keyboard::Keys key)
{
	auto kb = Elixir::InputManager::GetInstance().GetKeyboardState();
	return kb.IsKeyUp(key);
}

bool Elixir::Input::IsMouseDown(MouseButton button)
{
	auto mouse = Elixir::InputManager::GetInstance().GetMouseState();
	switch (button)
	{
	case MBLeft:
		return mouse.leftButton;
	case MBRight:
		return mouse.rightButton;
	case MBMiddle:
		return mouse.middleButton;
	}

	return false;
}
