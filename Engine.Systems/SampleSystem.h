#pragma once
#include "../ElixirEngine/System.h"
#include "Sample.h"

class SampleSystem : public Elixir::ISystem
{
	float totalTime;
	std::vector<Elixir::EntityID> entities;
	std::vector<TestA*> aData;
	std::vector<TestB*> bData;

public:
	// Inherited via ISystem
	virtual void Init()
	{
		RegisterComponent<TestA>();
		RegisterComponent<TestB>();
		totalTime = 0.f;
	}

	virtual void PreUpdate() override;
	virtual void Update(float deltaTime) override;
	virtual void PostUpdate() override;
};

