#pragma once
#include "../ElixirEngine/System.h"

class RotatorSystem : public Elixir::ISystem
{
	float totalTime = 0.f;
public:
	RotatorSystem();
	~RotatorSystem();

	// Inherited via ISystem
	virtual void Update(float deltaTime) override;
};

