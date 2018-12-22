#pragma once
#include "Core/Core.h"
#include "Texture.h"
#include "ComputeCore.h"
#include "ComputeProcess.h"
#include "BlurFilter.h"
#include "DepthOfFieldPass.h"
#include "SunRaysPass.h"
#include "TexturePool.h"
#include "EdgeFilter.h"
#include "CompositeTextures.h"
#include "ResourceManager.h"

class Game : public Core
{
protected:
	ResourceManager* resourceManager;
	ID3D12Resource* skyboxTexture;
	ID3D12Resource* skyboxIRTexture;
	ID3D12Resource* brdfLutTexture;
	ID3D12Resource* skyboxPreFilter;

	Texture*	skyTexture;
	Camera*		camera;

	TexturePool*	texturePool;
	ComputeCore*	computeCore;
	POINT			prevMousePos;
	BlurFilter*		blurFilter;

	std::vector<Entity*> selectedEntities;
	std::vector<std::unique_ptr<Entity>> entities;

	std::unique_ptr<DepthOfFieldPass>	dofPass;
	std::unique_ptr<SunRaysPass>		sunRaysPass;
	std::unique_ptr<EdgeFilter>			edgeFilter;
	std::unique_ptr<CompositeTextures>	compositeTextures;

	bool			isBlurEnabled;
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

