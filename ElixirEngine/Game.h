#pragma once
#include "Core/Core.h"
#include "Texture.h"
#include "ComputeProcess.h"

class Game : public Core
{
protected:
	ID3D12Resource* textureBuffer;
	ID3D12Resource* normalTexture;
	ID3D12Resource* roughnessTexture;
	ID3D12Resource* metalnessTexture;

	ID3D12Resource* skyboxTexture;
	ID3D12Resource* skyboxIRTexture;
	ID3D12Resource* brdfLutTexture;
	ID3D12Resource* skyboxPreFilter;

	Texture*		skyTexture;

	Mesh*		sphereMesh;
	Mesh*		cubeMesh;
	Camera*		camera;
	Entity*		entity1;
	Entity*		entity2;
	Entity*		entity3;
	Entity*		entity4;
	Material*	scratchedMaterial;
	Material*	breadMaterial;
	Material*	woodenMaterial;
	Material*	cobblestoneMaterial;
	Material*	bronzeMaterial;

	POINT prevMousePos;
	ComputeProcess* computeProcess;
	void InitializeAssets();
public:
	Game(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;

	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);
	~Game();
};

