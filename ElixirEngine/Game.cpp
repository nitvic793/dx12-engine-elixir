#include "stdafx.h"
#include "Game.h"

//Initializes assets. This function's scope has access to commandList which is not closed. 
void Game::InitializeAssets()
{
	int entityCount = 10;
	std::vector<std::wstring> textureList = {
			L"../../Assets/Textures/floor_albedo.png" ,
			L"../../Assets/Textures/floor_normals.png" ,
			L"../../Assets/Textures/floor_roughness.png" ,
			L"../../Assets/Textures/floor_metal.png",
			L"../../Assets/Textures/wood_albedo.png" ,
			L"../../Assets/Textures/wood_normals.png" ,
			L"../../Assets/Textures/wood_roughness.png" ,
			L"../../Assets/Textures/wood_metal.png",
			L"../../Assets/Textures/scratched_albedo.png" ,
			L"../../Assets/Textures/scratched_normals.png" ,
			L"../../Assets/Textures/scratched_roughness.png" ,
			L"../../Assets/Textures/scratched_metal.png",
			L"../../Assets/Textures/bronze_albedo.png" ,
			L"../../Assets/Textures/bronze_normals.png" ,
			L"../../Assets/Textures/bronze_roughness.png" ,
			L"../../Assets/Textures/bronze_metal.png"
	};

	std::vector<std::string> meshList = {
		"../../Assets/sphere.obj"
	};

	size_t materialCount = textureList.size() / 4;
	for (int i = 0; i < textureList.size(); i += 4)
	{
		materials.push_back(std::unique_ptr<Material>(new Material(
			deferredRenderer,
			{
				textureList[i].c_str(),
				textureList[i + 1].c_str(),
				textureList[i + 2].c_str(),
				textureList[i + 3].c_str()
			}, 
			device, 
			commandQueue)));
	}

	for (int i = 0; i < meshList.size(); ++i)
	{
		meshes.push_back(std::unique_ptr<Mesh>(new Mesh(meshList[i], device, commandList)));
	}

	std::vector<int> entityMaterialMap;
	for (int i = 0; i < entityCount; ++i)
	{
		entityMaterialMap.push_back(i % materialCount);
	}

	texturePool = new TexturePool(device, deferredRenderer);
	isBlurEnabled = false;
	computeCore = new ComputeCore(device);

	dofPass = std::unique_ptr<DepthOfFieldPass>(new DepthOfFieldPass(computeCore));
	blurFilter = new BlurFilter(computeCore);
	camera = new Camera((float)Width, (float)Height);

	pixelCb.light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light.DiffuseColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.f);
	pixelCb.light.Direction = XMFLOAT3(-1.f, 0.f, 1.f);
	pixelCb.pointLight = PointLight{ {0.99f, 0.2f, 0.2f, 0.f} , {0.0f, 2.f, 1.f}, 20.f };

	ResourceUploadBatch uploadBatch(device);
	uploadBatch.Begin();
	CreateDDSTextureFromFile(device, uploadBatch, L"../../Assets/envEnvHDR.dds", &skyboxTexture);
	CreateDDSTextureFromFile(device, uploadBatch, L"../../Assets/envDiffuseHDR.dds", &skyboxIRTexture);
	CreateDDSTextureFromFile(device, uploadBatch, L"../../Assets/envBrdf.dds", &brdfLutTexture);
	CreateDDSTextureFromFile(device, uploadBatch, L"../../Assets/envSpecularHDR.dds", &skyboxPreFilter);
	auto uploadOperation = uploadBatch.End(commandQueue);
	uploadOperation.wait();

	for (int i = 0; i < entityCount; ++i)
	{
		entities.push_back(std::unique_ptr<Entity>(new Entity()));
		entities[i]->SetMesh(meshes[0].get());
		auto pos = XMFLOAT3((float)i, 0, 0);
		entities[i]->SetPosition(pos);
		int matId = entityMaterialMap[i];
		entities[i]->SetMaterial(materials[matId].get());
	}

	deferredRenderer->SetIBLTextures(skyboxIRTexture, skyboxPreFilter, brdfLutTexture);
	skyTexture = new Texture(deferredRenderer, device);
	skyTexture->CreateTexture(L"../../Assets/envEnvHDR.dds", TexFileTypeDDS, commandQueue, true);
}

Game::Game(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen) :
	Core(hInstance, ShowWnd, width, height, fullscreen)
{
}

void Game::Initialize()
{
	InitializeResources();
	InitializeAssets();
	EndInitialization();
}

float CurrentTime = 0.f;

void Game::Update()
{
	CurrentTime += deltaTime;
	camera->Update(deltaTime);
	if (GetAsyncKeyState(VK_TAB))
	{
		isBlurEnabled = true;
	}
	else
	{
		isBlurEnabled = false;
	}
}

void Game::Draw()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pixelCb.cameraPosition = camera->GetPosition();
	pixelCb.invProjView = camera->GetInverseProjectionViewMatrix();

	std::vector<Entity*> entityList;
	for (auto& entity : entities)
	{
		entityList.push_back(entity.get());
	}

	// draw
	deferredRenderer->SetGBUfferPSO(commandList, camera, pixelCb);
	deferredRenderer->Draw(commandList, entityList
	);

	deferredRenderer->SetLightShapePassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightShapePass(commandList, pixelCb);

	deferredRenderer->SetLightPassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightPass(commandList);

	deferredRenderer->DrawSkybox(commandList, skyTexture);

	Texture* finalTexture = deferredRenderer->GetResultSRV();

	if (isBlurEnabled)
	{
		auto blurTexture = blurFilter->Apply(commandList, finalTexture, texturePool, 4, 6, 2);
		finalTexture = dofPass->Apply(commandList, finalTexture, blurTexture, texturePool, 6, 0.2f);
	}

	deferredRenderer->DrawResult(commandList, rtvHandle, finalTexture); //Draw renderer result to given main Render Target handle
	deferredRenderer->ResetRenderTargetStates(commandList);
}

void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;
	SetCapture(hwnd);
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
	delete texturePool;
	delete blurFilter;
	delete computeCore;
	delete camera;
	delete skyTexture;

	skyboxTexture->Release();
	skyboxIRTexture->Release();
	brdfLutTexture->Release();
	skyboxPreFilter->Release();
}
