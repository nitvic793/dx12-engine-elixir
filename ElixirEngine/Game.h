#pragma once
#include "Core/Core.h"

class Game : public Core
{
protected:
	ID3D12Resource* textureBuffer; // the resource heap containing our texture
	ID3D12Resource* normalTexture; // the resource heap containing our texture'

	Mesh* mesh;
	Camera* camera;
	Entity* entity1;
	Entity* entity2;
	Entity* entity3;

	void InitializeAssets();
public:
	Game(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;
	~Game();
};

