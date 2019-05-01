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
	entity->SetRotation(0, XMFLOAT3(0, sin(totalTime), 0));
	entity->SetPosition(0, XMFLOAT3(2 * sin(totalTime) + 2, 1.f, cos(totalTime)));
	totalTime += deltaTime;
}
