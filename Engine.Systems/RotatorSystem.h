#pragma once
#include "../ElixirEngine/System.h"
#include "../ElixirEngine/Input.h"

class RotatorSystem : public Elixir::ISystem
{
	float totalTime = 0.f;
	float currentTime = 0.f;
	bool toggle = true;
public:
	RotatorSystem();
	~RotatorSystem();

	// Inherited via ISystem
	virtual void Update(float deltaTime) override;
};

