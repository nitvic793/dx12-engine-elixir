#include "stdafx.h"
#include "Game.h"
#include "DirectXMesh.h"
#include "Model.h"
#include "AnimationSystem.h"
#include "ModelLoader.h"
#include "Utility.h"

//Initializes assets. This function's scope has access to commandList which is not closed. 
void Game::InitializeAssets()
{
	deferredRenderer->SetAnimationManager(animationManager.get());
	deferredRenderer->SetEntityManager(&entityManager);
	auto rm = resourceManager;
	rm->Initialize(animationManager.get(), &entityManager);
	rm->LoadResources("../../SceneData/resources.json", commandQueue, commandList, deferredRenderer);
	rm->LoadScene("../../SceneData/scene.json", entities);

	instanced = new MeshInstanceGroupEntity(
		{ StringID("sphere") }, { StringID("scratched") },
		{
			XMFLOAT3(-1,1,5),
			XMFLOAT3(5,0,-4),
			XMFLOAT3(6,0,-4),
			XMFLOAT3(7,0,-4),
			XMFLOAT3(8,0,-4),
			XMFLOAT3(9,0,-4),
			XMFLOAT3(10,0,-4),
		},
		device, commandList);


	texturePool = new TexturePool(device, deferredRenderer, 24);
	isBlurEnabled = false;
	computeCore = new ComputeCore(device, deferredRenderer);

	dofPass = std::unique_ptr<DepthOfFieldPass>(new DepthOfFieldPass(computeCore));
	sunRaysPass = std::unique_ptr<SunRaysPass>(new SunRaysPass(computeCore, deferredRenderer));
	edgeFilter = std::unique_ptr<EdgeFilter>(new EdgeFilter(computeCore));
	blurFilter = new BlurFilter(computeCore);
	compositeTextures = std::unique_ptr<CompositeTextures>(new CompositeTextures(computeCore));
	downScaler = std::unique_ptr<DownScaleTexture>(new DownScaleTexture(computeCore));
	camera = new Camera((float)Width, (float)Height);

	pixelCb.light[0] = DirectionalLight();
	pixelCb.light[0].AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light[0].DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.f);
	pixelCb.light[0].Intensity = 0.2f;
	pixelCb.light[0].Direction = XMFLOAT3(-0.3f, -0.2f, -1.f);
	pixelCb.light[1] = DirectionalLight();
	pixelCb.light[1].AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light[1].DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.f);
	pixelCb.light[1].Intensity = 0.2f;
	pixelCb.light[1].Direction = XMFLOAT3(0.3f, -0.5f, 1.f);

	pixelCb.dirLightCount = 1;
	pixelCb.dirLightIndex = 0;

	pixelCb.pointLight[0] = PointLight{ {0.99f, 0.2f, 0.2f, 0.f} , {0.0f, 0.0f, -1.f}, 16.f , 2.f };
	pixelCb.pointLight[1] = PointLight{ {0.0f, 0.99f, 0.2f, 0.f} , {5.0f, 0.0f, -1.f}, 6.f , 2.f };
	pixelCb.pointLightCount = 2u;
	pixelCb.pointLightIndex = 0;

	deferredRenderer->SetIBLTextures(
		rm->GetTexture(StringID("Irradiance"))->GetTextureResource(),
		rm->GetTexture(StringID("Prefilter"))->GetTextureResource(),
		rm->GetTexture(StringID("Brdf"))->GetTextureResource()
	);

	skyTexture = rm->GetTexture(StringID("skybox"));

	auto eId = entityManager.CreateEntity(0, "Test", StringID("sphere"), StringID("floor"), Elixir::Transform::Create(XMFLOAT3(-1, 0, 0)));
	auto e2 = entityManager.CreateEntity(eId, "Test", StringID("sphere"), StringID("bronze"), Elixir::Transform::Create(XMFLOAT3(-1, 0, 0)));

	entityManager.AddComponent<Elixir::TestA>(eId, {1.f});
	entityManager.AddComponent(1, Elixir::TestA {0.3f});
	entityManager.AddComponent<Elixir::TestB>(eId);
	entityManager.AddComponent<Elixir::TestB>(1);
	entityManager.AddComponent<AnimationComponent>(8);
	systemManager.RegisterSystem<Elixir::SampleSystem>();
	systemManager.RegisterSystem<AnimationSystem>(animationManager.get());
	systemManager.Init();
}

Game::Game(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen) :
	Core(hInstance, ShowWnd, width, height, fullscreen),
	entityManager(&scene),
	systemManager(&entityManager)
{
	resourceManager = ResourceManager::CreateInstance(device);
	animationManager = std::make_unique<AnimationManager>();
}

void Game::Initialize()
{
	InitializeResources();
	InitializeAssets();
	EndInitialization();
}

float CurrentTime = 0.f;
float delay = 0.2f;
int animIndex = 0;
bool debugMode = false;
int gBufferIndex = RTV_ORDER_ALBEDO;

void Game::Update()
{
	systemManager.Update(deltaTime);
	auto& entity = entityManager;
	CurrentTime += deltaTime;
	camera->Update(deltaTime);
	entity.SetRotation(0, XMFLOAT3(0, sin(totalTime), 0));
	entity.SetPosition(0, XMFLOAT3(2 * sin(totalTime) + 2, 1.f, cos(totalTime)));
	entity.SetRotation(8, XMFLOAT3(XM_PIDIV2, 0, 0));

	pixelCb.pointLight[1].Position = XMFLOAT3(2 * sin(totalTime * 2) + 1, 0, -1);
	pixelCb.pointLight[0].Position = XMFLOAT3(2 * sin(totalTime * 2) + 5, 1.0f, -2 + -2 * cos(totalTime));
	if (GetAsyncKeyState('Q'))
	{
		isBlurEnabled = true;
	}
	else
	{
		isBlurEnabled = false;
	}

	if (GetAsyncKeyState(VK_UP) & 0xFFFF8000 && CurrentTime > delay)
	{
		animIndex++;
		if (animIndex > 6) animIndex = 6;
		entityManager.GetComponent<AnimationComponent>(8).CurrentAnimationIndex = animIndex;
		CurrentTime = 0.f;
	}

	if (GetAsyncKeyState(VK_DOWN) & 0xFFFF8000 && CurrentTime > delay)
	{
		animIndex--;
		if (animIndex < 0)animIndex = 0;
		CurrentTime = 0.f;
		entityManager.GetComponent<AnimationComponent>(8).CurrentAnimationIndex = animIndex;
	}

	if (GetAsyncKeyState(192) & 0xFFFF8000 && CurrentTime > delay)
	{
		debugMode = !debugMode;
		CurrentTime = 0.f;
	}

	if (GetAsyncKeyState(VK_ADD) & 0xFFFF8000 && CurrentTime > delay)
	{
		gBufferIndex = (gBufferIndex + 1) % RTV_ORDER_COUNT;
		CurrentTime = 0.f;
	}

	scene.UpdateTransforms();
}


void Game::Draw()
{
	//Update CB
	pixelCb.cameraPosition = camera->GetPosition();
	pixelCb.invProjView = camera->GetInverseProjectionViewMatrix();

	deferredRenderer->StartFrame(commandList);
	resourceManager->GetMesh(StringID("man"))->BoneTransform(0, totalTime, 0);
	//entities[8]->UpdateAnimation(totalTime, animIndex);

	std::vector<Elixir::Entity> eEntities;
	entityManager.GetEntities(eEntities);

	deferredRenderer->PrepareFrame(eEntities, camera, pixelCb);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Render shadow Map before setting viewport and scissor rect
	deferredRenderer->RenderShadowMap(commandList, eEntities, { instanced }); //TODO: shadows for instanced objects

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	// draw
	deferredRenderer->RenderSelectionDepthBuffer(commandList, selectedEntities, camera);
	deferredRenderer->SetGBUfferPSO(commandList, camera, pixelCb);
	deferredRenderer->Draw(commandList, eEntities);
	deferredRenderer->DrawAnimated(commandList, eEntities);
	deferredRenderer->DrawInstanced(commandList, { instanced });

	deferredRenderer->RenderLightShapePass(commandList, pixelCb);
	deferredRenderer->RenderLightPass(commandList, pixelCb);
	deferredRenderer->RenderAmbientPass(commandList);
	deferredRenderer->DrawSkybox(commandList, skyTexture);
	deferredRenderer->TransitionToPostProcess(commandList);

	auto finalTexture = deferredRenderer->GetResultSRV();
	auto downscaled = downScaler->Apply(commandList, finalTexture, texturePool);
	if (isBlurEnabled)
	{
		float focusPlane = 8.f;
		auto blurTexture = blurFilter->Apply(commandList, downscaled, texturePool, 4, focusPlane, 2);
		finalTexture = dofPass->Apply(commandList, finalTexture, blurTexture, texturePool, focusPlane, 0.05f);
	}

	finalTexture = sunRaysPass->Apply(commandList, deferredRenderer->GetGBufferDepthSRV(), finalTexture, texturePool, camera);
	//finalTexture = edgeFilter->Apply(commandList, deferredRenderer->GetSelectionDepthBufferSRV(), finalTexture, texturePool);
	//finalTexture = compositeTextures->Composite(commandList, finalTexture, deferredRenderer->GetSelectionOutlineSRV(), texturePool);

	if (!debugMode)
	{
		deferredRenderer->DrawResult(commandList, rtvHandle, finalTexture); //Draw renderer result to given main Render Target handle
	}
	else
	{
		deferredRenderer->DrawResult(commandList, rtvHandle, deferredRenderer->GetGBufferTextureSRV((GBufferType)gBufferIndex));
	}

	deferredRenderer->EndFrame(commandList);
}

void Game::Shutdown()
{
	systemManager.Shutdown();
	for (auto& e : entities) delete e;
	delete texturePool;
	delete blurFilter;
	delete computeCore;
	delete camera;
	delete instanced;
	delete resourceManager;
}

void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;
	SetCapture(hwnd);
	selectedEntities.clear();
	for (int i = 0; i < entities.size(); ++i)
	{
		float distance;
		if (IsIntersecting(entities[i]->GetBoundingBox(), camera, x, y, distance))
		{
			selectedEntities.push_back(entities[i]);
			printf("Intersecting %d\n", i);
			break;
		}
	}

}

void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	if (buttonState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * 0.005f;
		float yDiff = (y - prevMousePos.y) * 0.005f;
		camera->Rotate(yDiff, xDiff);
	}

	prevMousePos.x = x;
	prevMousePos.y = y;
}

void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
}

Game::~Game()
{
}
