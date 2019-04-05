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
#include "DownScaleTexture.h"
#include "MeshInstanceGroupEntity.h"
#include "AnimationManager.h"

class Game : public Core
{
protected:
	ResourceManager*					resourceManager;
	std::unique_ptr<AnimationManager>	animationManager;

	Texture*	skyTexture;
	Camera*		camera;

	TexturePool*	texturePool;
	ComputeCore*	computeCore;
	POINT			prevMousePos;
	
	std::vector<Entity*> selectedEntities;
	std::vector<Entity*> entities;
	MeshInstanceGroupEntity* instanced;

	BlurFilter*							blurFilter;
	std::unique_ptr<DepthOfFieldPass>	dofPass;
	std::unique_ptr<SunRaysPass>		sunRaysPass;
	std::unique_ptr<EdgeFilter>			edgeFilter;
	std::unique_ptr<CompositeTextures>	compositeTextures;
	std::unique_ptr<DownScaleTexture>	downScaler;

	bool isBlurEnabled;
	void InitializeAssets();
public:
	Game(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);
	virtual void Initialize() override;
	virtual void Update() override;
	virtual void Draw() override;

	void Shutdown();
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);
	~Game();
};

