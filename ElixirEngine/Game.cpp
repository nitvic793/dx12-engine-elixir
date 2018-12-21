#include "stdafx.h"
#include "Game.h"
#include "StringHash.h"

//Initializes assets. This function's scope has access to commandList which is not closed. 
void Game::InitializeAssets()
{
	int entityCount = 10;
	std::vector<std::wstring> textureList = {
			L"../../Assets/Textures/floor_albedo.png" , //0
			L"../../Assets/Textures/floor_normals.png" ,
			L"../../Assets/Textures/floor_roughness.png" ,
			L"../../Assets/Textures/floor_metal.png",
			L"../../Assets/Textures/wood_albedo.png" , //1
			L"../../Assets/Textures/wood_normals.png" ,
			L"../../Assets/Textures/wood_roughness.png" ,
			L"../../Assets/Textures/wood_metal.png",
			L"../../Assets/Textures/scratched_albedo.png" , //2
			L"../../Assets/Textures/scratched_normals.png" ,
			L"../../Assets/Textures/scratched_roughness.png" ,
			L"../../Assets/Textures/scratched_metal.png",
			L"../../Assets/Textures/bronze_albedo.png" , //3
			L"../../Assets/Textures/bronze_normals.png" ,
			L"../../Assets/Textures/bronze_roughness.png" ,
			L"../../Assets/Textures/bronze_metal.png",
			L"../../Assets/Textures/cement_albedo.png" , //4
			L"../../Assets/Textures/cement_normals.png" ,
			L"../../Assets/Textures/cement_roughness.png" ,
			L"../../Assets/Textures/cement_metal.png",
			L"../../Assets/Textures/asw_albedo.png" , //5
			L"../../Assets/Textures/marble_normals.jpg" ,
			L"../../Assets/Textures/marble_roughness.jpg",
			L"../../Assets/Textures/marble_metal.png",
			L"../../Assets/Textures/hammer_albedo.png" , //6
			L"../../Assets/Textures/hammer_normals.png" ,
			L"../../Assets/Textures/cement_roughness.png",
			L"../../Assets/Textures/hammer_metal.png"
	};

	std::vector<std::string> meshList = {
		"../../Assets/sphere.obj",
		"../../Assets/quad.obj",
		"../../Assets/sw.obj",
		"../../Assets/hammer.obj",
	};

	size_t materialCount = 3;
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

	texturePool = new TexturePool(device, deferredRenderer, 12);
	isBlurEnabled = false;
	computeCore = new ComputeCore(device);

	dofPass = std::unique_ptr<DepthOfFieldPass>(new DepthOfFieldPass(computeCore));
	sunRaysPass = std::unique_ptr<SunRaysPass>(new SunRaysPass(computeCore, deferredRenderer));
	edgeFilter = std::unique_ptr<EdgeFilter>(new EdgeFilter(computeCore));
	blurFilter = new BlurFilter(computeCore);
	compositeTextures = std::unique_ptr<CompositeTextures>(new CompositeTextures(computeCore));
	camera = new Camera((float)Width, (float)Height);

	pixelCb.light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light.DiffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.f);
	pixelCb.light.Intensity = 1.0f;
	pixelCb.light.Direction = XMFLOAT3(0.3f, -0.5f, -1.f);
	pixelCb.pointLight[0] = PointLight{ {0.99f, 0.2f, 0.2f, 0.f} , {0.0f, 0.0f, -1.f}, 6.f };
	pixelCb.pointLight[1] = PointLight{ {0.0f, 0.99f, 0.2f, 0.f} , {5.0f, 0.0f, -1.f}, 6.f };
	pixelCb.pointLightCount = 2u;
	pixelCb.pointLightIndex = 0;

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
		entities[i]->SetCastsShadow(true);
	}

	entities[8]->SetMesh(meshes[3].get());
	entities[7]->SetMesh(meshes[2].get());
	entities[6]->SetMesh(meshes[2].get());

	entities[8]->SetRotation(XMFLOAT3(-XM_PIDIV2 / 4, -XM_PIDIV2 / 4, 0.f));
	entities[8]->SetPosition(XMFLOAT3(0, -1, 0));
	entities[7]->SetPosition(XMFLOAT3(15, 1, 5));
	entities[6]->SetPosition(XMFLOAT3(0, 2, 5));

	//entities[8]->SetScale(XMFLOAT3(0.01f, 0.01f, 0.01f));
	entities[7]->SetScale(XMFLOAT3(0.01f, 0.01f, 0.01f));
	entities[6]->SetScale(XMFLOAT3(0.01f, 0.01f, 0.01f));

	entities[8]->SetMaterial(materials[6].get());
	entities[7]->SetMaterial(materials[5].get());
	entities[6]->SetMaterial(materials[5].get());

	entities[9]->SetCastsShadow(false);
	entities[9]->SetMesh(meshes[1].get());
	entities[9]->SetPosition(XMFLOAT3(5, -1, 0));
	entities[9]->SetScale(XMFLOAT3(15, 15, 15));
	entities[9]->SetMaterial(materials[4].get());

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
	entities[0]->SetPosition(XMFLOAT3(2 * sin(totalTime) + 2, 1.f, cos(totalTime)));
	pixelCb.pointLight[1].Position = XMFLOAT3(2 * sin(totalTime * 2) + 5, 0, -1);
	pixelCb.pointLight[0].Position = XMFLOAT3(2 * cos(totalTime * 2), 0, -1);
	if (GetAsyncKeyState(VK_TAB))
	{
		isBlurEnabled = true;
	}
	else
	{
		isBlurEnabled = false;
	}
}

bool IsIntersecting(Entity* entity, Camera* camera, int mouseX, int mouseY, float& distance)
{
	uint16_t screenWidth = 1280;
	uint16_t screenHeight = 720;
	auto viewMatrix = XMLoadFloat4x4(&camera->GetViewMatrix());
	auto projMatrix = XMLoadFloat4x4(&camera->GetProjectionMatrix());

	auto orig = XMVector3Unproject(XMVectorSet((float)mouseX, (float)mouseY, 0.f, 0.f),
		0,
		0,
		screenWidth,
		screenHeight,
		0,
		1,
		projMatrix,
		viewMatrix,
		XMMatrixIdentity());

	auto dest = XMVector3Unproject(XMVectorSet((float)mouseX, (float)mouseY, 1.f, 0.f),
		0,
		0,
		screenWidth,
		screenHeight,
		0,
		1,
		projMatrix,
		viewMatrix,
		XMMatrixIdentity());

	auto direction = dest - orig;
	direction = XMVector3Normalize(direction);
	bool intersecting = false;// entity->GetBoundingSphere().Intersects(orig, direction, distance);
	//bool intersecting = /*intersecting ||*/ entity->GetBoundingBox().Intersects(orig, direction, distance);
	return intersecting;
}

void Game::Draw()
{
	std::vector<Entity*> entityList;
	for (auto& entity : entities)
	{
		entityList.push_back(entity.get());
	}

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Render shadow Map before setting viewport and scissor rect
	deferredRenderer->RenderShadowMap(commandList, entityList);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	pixelCb.cameraPosition = camera->GetPosition();
	pixelCb.invProjView = camera->GetInverseProjectionViewMatrix();

	// draw
	deferredRenderer->RenderSelectionDepthBuffer(commandList, selectedEntities, camera);
	deferredRenderer->SetGBUfferPSO(commandList, camera, pixelCb);
	deferredRenderer->Draw(commandList, entityList);

	deferredRenderer->RenderLightShapePass(commandList, pixelCb);
	deferredRenderer->RenderLightPass(commandList, pixelCb);
	deferredRenderer->DrawSkybox(commandList, skyTexture);

	Texture* finalTexture = deferredRenderer->GetResultSRV();

	if (isBlurEnabled)
	{
		auto blurTexture = blurFilter->Apply(commandList, finalTexture, texturePool, 4, 3, 2);
		finalTexture = dofPass->Apply(commandList, finalTexture, blurTexture, texturePool, 3, 0.2f);
	}
	finalTexture = sunRaysPass->Apply(commandList, deferredRenderer->GetGBufferDepthSRV(), finalTexture, texturePool, camera);
	//finalTexture = edgeFilter->Apply(commandList, deferredRenderer->GetSelectionDepthBufferSRV(), finalTexture, texturePool);
	finalTexture = compositeTextures->Composite(commandList, finalTexture, deferredRenderer->GetSelectionOutlineSRV(), texturePool);

	deferredRenderer->DrawResult(commandList, rtvHandle, finalTexture); //Draw renderer result to given main Render Target handle

	deferredRenderer->ResetRenderTargetStates(commandList);
	texturePool->ResetIndex();
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
		if (IsIntersecting(entities[i].get(), camera, x, y, distance))
		{
			selectedEntities.push_back(entities[i].get());
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
