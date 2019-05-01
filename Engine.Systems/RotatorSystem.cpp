#include "stdafx.h"
#include "RotatorSystem.h"


RotatorSystem::RotatorSystem()
{
}


RotatorSystem::~RotatorSystem()
{
}

void RotatorSystem::Update(float deltaTime)
{
	if (toggle)
	{
		entity->SetRotation(0, XMFLOAT3(0, sin(totalTime), 0));
		entity->SetPosition(0, XMFLOAT3(2 * sin(totalTime) + 2, 1.f, cos(totalTime)));
		totalTime += deltaTime;
	}

	if (context.Input->IsKeyDown(DirectX::Keyboard::T) && currentTime > 0.1f)
	{
		currentTime = 0.f;
		toggle = !toggle;
	}

	currentTime += deltaTime;
}
