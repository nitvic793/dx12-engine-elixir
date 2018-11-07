#include "stdafx.h"
#include "Game.h"

//Initializes assets. This function's scope has access to commandList which is not closed. 
void Game::InitializeAssets()
{
	computeProcess = new ComputeProcess(device, L"ComputeCS.cso");
	camera = new Camera((float)Width, (float)Height);
	entity1 = new Entity();
	entity2 = new Entity();
	entity3 = new Entity();
	entity4 = new Entity();
	sphereMesh = new Mesh("../../Assets/sphere.obj", device, commandList);
	cubeMesh = new Mesh("../../Assets/wrench.obj", device, commandList);
	entity1->SetMesh(sphereMesh);
	entity2->SetMesh(cubeMesh);
	entity2->SetScale(XMFLOAT3(0.05f, 0.05f, 0.05f));
	entity2->SetRotation(XMFLOAT3(45 * XM_PIDIV2, 0, 0));
	entity3->SetMesh(sphereMesh);
	entity4->SetMesh(sphereMesh);

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

	scratchedMaterial = new Material(
		deferredRenderer,
		{
			L"../../Assets/Textures/floor_albedo.png" ,
			L"../../Assets/Textures/floor_normals.png" ,
			L"../../Assets/Textures/floor_roughness.png" ,
			L"../../Assets/Textures/floor_metal.png"
		},
		device,
		commandQueue
	);

	woodenMaterial = new Material(
		deferredRenderer,
		{
			L"../../Assets/Textures/wood_albedo.png" ,
			L"../../Assets/Textures/wood_normals.png" ,
			L"../../Assets/Textures/wood_roughness.png" ,
			L"../../Assets/Textures/wood_metal.png"
		},
		device,
		commandQueue
	);

	cobblestoneMaterial = new Material(
		deferredRenderer,
		{
			L"../../Assets/Textures/rough_albedo.png" ,
			L"../../Assets/Textures/rough_normals.png" ,
			L"../../Assets/Textures/rough_roughness.png" ,
			L"../../Assets/Textures/rough_metal.png"
		},
		device,
		commandQueue
	);

	bronzeMaterial = new Material(
		deferredRenderer,
		{
			L"../../Assets/Textures/bronze_albedo.png" ,
			L"../../Assets/Textures/bronze_normals.png" ,
			L"../../Assets/Textures/bronze_roughness.png" ,
			L"../../Assets/Textures/bronze_metal.png"
		},
		device,
		commandQueue
	);

	breadMaterial = new Material(
		deferredRenderer,
		{
			L"../../Assets/Textures/wrench_albedo.jpg" ,
			L"../../Assets/Textures/wrench_normals.png" ,
			L"../../Assets/Textures/wrench_roughness.jpg" ,
			L"../../Assets/Textures/wrench_metal.jpg"
		},
		device,
		commandQueue
	);

	deferredRenderer->SetIBLTextures(skyboxIRTexture, skyboxPreFilter, brdfLutTexture);

	skyTexture = new Texture(deferredRenderer, device);
	skyTexture->CreateTexture(L"../../Assets/envEnvHDR.dds", TexFileTypeDDS, commandQueue, true);

	entity1->SetMaterial(scratchedMaterial);
	entity2->SetMaterial(breadMaterial);
	entity3->SetMaterial(cobblestoneMaterial);
	entity4->SetMaterial(bronzeMaterial);
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
	auto pos = XMFLOAT3(-2, 0, 2);
	entity1->SetPosition(pos);
	entity2->SetPosition(XMFLOAT3(2, 0, 2));
	entity3->SetPosition(XMFLOAT3(0, 0, 5));
	/*float rotX = 2 * sin(totalTime);
	entity1->SetRotation(XMFLOAT3(rotX, rotX, 0));*/
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

	// draw
	deferredRenderer->SetGBUfferPSO(commandList, camera, pixelCb);
	deferredRenderer->Draw(commandList, {
			entity1,
			entity2,
			entity3,
			entity4
		}
	);

	deferredRenderer->SetLightShapePassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightShapePass(commandList, pixelCb);

	deferredRenderer->SetLightPassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightPass(commandList);

	deferredRenderer->DrawSkybox(commandList, skyTexture);
	computeProcess->SetShader(commandList);
	computeProcess->SetTextureUAV(commandList, deferredRenderer->GetResultUAV());
	computeProcess->Dispatch(commandList, 8);
	deferredRenderer->DrawResult(commandList, rtvHandle); //Draw renderer result to given main Render Target handle

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
	delete computeProcess;
	delete sphereMesh;
	delete cubeMesh;
	delete camera;
	delete entity1;
	delete entity2;
	delete entity3;
	delete entity4;
	delete scratchedMaterial;
	delete woodenMaterial;
	delete cobblestoneMaterial;
	delete bronzeMaterial;
	delete breadMaterial;
	delete skyTexture;

	skyboxTexture->Release();
	skyboxIRTexture->Release();
	brdfLutTexture->Release();
	skyboxPreFilter->Release();
}
