#include "stdafx.h"
#include "Game.h"

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
			L"../../Assets/Textures/cement_metal.png"
	};

	std::vector<std::string> meshList = {
		"../../Assets/sphere.obj",
		"../../Assets/quad.obj"
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

	texturePool = new TexturePool(device, deferredRenderer);
	isBlurEnabled = false;
	computeCore = new ComputeCore(device);

	dofPass = std::unique_ptr<DepthOfFieldPass>(new DepthOfFieldPass(computeCore));
	sunRaysPass = std::unique_ptr<SunRaysPass>(new SunRaysPass(computeCore, deferredRenderer));
	blurFilter = new BlurFilter(computeCore);
	camera = new Camera((float)Width, (float)Height);

	pixelCb.light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light.DiffuseColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.f);
	pixelCb.light.Direction = XMFLOAT3(0.5f, 0.5f, 1.f);
	pixelCb.pointLight = PointLight{ {0.99f, 0.2f, 0.2f, 0.f} , {0.0f, 2.f, 1.f}, 10.f };

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
	if (GetAsyncKeyState(VK_TAB))
	{
		isBlurEnabled = true;
	}
	else
	{
		isBlurEnabled = false;
	}
}

bool IsIntersecting(Entity* entity, Camera* camera, int mouseX, int mouseY)
{
	auto projMatrix = camera->GetProjectionMatrix();
	auto pointX = ((2.0f * (float)mouseX) / (float)1280) - 1.0f;
	auto pointY = (((2.0f * (float)mouseY) / (float)720) - 1.0f) * -1.0f;

	pointX = pointX / projMatrix._11;
	pointY = pointY / projMatrix._22;
	
	auto viewMatrix = camera->GetViewMatrix();
	XMFLOAT4X4 inverseViewMatrix;
	auto invView = XMMatrixInverse(nullptr, XMLoadFloat4x4(&viewMatrix));
	XMStoreFloat4x4(&inverseViewMatrix, invView);
	XMFLOAT3 direction;

	// Calculate the direction of the picking ray in view space.
	direction.x = (pointX * inverseViewMatrix._11) + (pointY * inverseViewMatrix._21) + inverseViewMatrix._31;
	direction.y = (pointX * inverseViewMatrix._12) + (pointY * inverseViewMatrix._22) + inverseViewMatrix._32;
	direction.z = (pointX * inverseViewMatrix._13) + (pointY * inverseViewMatrix._23) + inverseViewMatrix._33;

	// Get the origin of the picking ray which is the position of the camera.
	auto origin = camera->GetPosition();

	// Get the world matrix and translate to the location of the sphere.
	auto worldMatrix = entity->GetWorldMatrix();
	auto world = XMLoadFloat4x4(&worldMatrix);
	auto invWorld = XMMatrixInverse(nullptr, world);
	// Now get the inverse of the translated world matrix.

	auto rayOrigin = XMVector3TransformCoord(XMLoadFloat3(&origin), invWorld);
	auto rayDirection = XMVector3TransformCoord(XMLoadFloat3(&direction), invWorld);
	// Now transform the ray origin and the ray direction from view space to world space.


	// Normalize the ray direction.
	rayDirection = XMVector3Normalize(rayDirection);
	float distance = 0.f;
	return entity->GetBoundingBox().Intersects(rayOrigin, rayDirection, distance);
	// Now perform the ray-sphere intersection test.

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
	deferredRenderer->SetGBUfferPSO(commandList, camera, pixelCb);
	deferredRenderer->Draw(commandList, entityList);

	deferredRenderer->SetLightShapePassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightShapePass(commandList, pixelCb);

	deferredRenderer->SetLightPassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightPass(commandList);

	deferredRenderer->DrawSkybox(commandList, skyTexture);

	Texture* finalTexture = deferredRenderer->GetResultSRV();

	if (isBlurEnabled)
	{
		auto blurTexture = blurFilter->Apply(commandList, finalTexture, texturePool, 4, 3, 2);
		finalTexture = dofPass->Apply(commandList, finalTexture, blurTexture, texturePool, 3, 0.2f);
	}
	auto sunRaysTex = sunRaysPass->Apply(commandList, deferredRenderer->GetGBufferDepthSRV(), finalTexture, texturePool, camera);
	deferredRenderer->DrawResult(commandList, rtvHandle, sunRaysTex); //Draw renderer result to given main Render Target handle
	deferredRenderer->ResetRenderTargetStates(commandList);
}

void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;
	SetCapture(hwnd);

	for (int i = 0; i < entities.size(); ++i)
	{
		if (IsIntersecting(entities[i].get(), camera, x, y))
		{
			printf("Intersecting %d\n", i);
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
