#include "stdafx.h"
#include "Game.h"

//Initializes assets. This function's scope has access to commandList which is not closed. 
void Game::InitializeAssets()
{
	camera = new Camera((float)Width, (float)Height);
	entity1 = new Entity();
	entity2 = new Entity();
	entity3 = new Entity();
	sphereMesh = new Mesh("../../Assets/sphere.obj", device, commandList);
	cubeMesh = new Mesh("../../Assets/torus.obj", device, commandList);
	entity1->SetMesh(cubeMesh);
	entity2->SetMesh(sphereMesh);
	entity3->SetMesh(sphereMesh);

	pixelCb.light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light.DiffuseColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.f);
	pixelCb.light.Direction = XMFLOAT3(-1.f, 0.f, 1.f);
	pixelCb.pointLight = PointLight{ {0.1f, 0.5f, 0.1f, 0.f} , {0.0f, 2.f, 0.f}, 10.f };

	ResourceUploadBatch uploadBatch(device);
	uploadBatch.Begin();
	CreateDDSTextureFromFile(device, uploadBatch, L"../../Assets/skybox2.dds", &skyboxTexture);
	CreateDDSTextureFromFile(device, uploadBatch, L"../../Assets/skybox2IR.dds", &skyboxIRTexture);
	CreateWICTextureFromFile(device, uploadBatch, L"../../Assets/ibl_brdf_lut.png", &brdfLutTexture);
	auto uploadOperation = uploadBatch.End(commandQueue);
	uploadOperation.wait();

	//auto image = std::make_unique<ScratchImage>();
	//LoadFromHDRFile(L"../../Assets/HDR/GravelPlaza_Env.hdr", nullptr, *image);
	
	scratchedMaterial = new Material(
		deferredRenderer->GetSRVHeap(), 
		{ 
			L"../../Assets/Textures/scratched_albedo.png" ,
			L"../../Assets/Textures/scratched_normals.png" ,
			L"../../Assets/Textures/scratched_roughness.png" ,
			L"../../Assets/Textures/scratched_metal.png" 
		},
		device, 
		commandQueue, 
		0
	);

	woodenMaterial = new Material(
		deferredRenderer->GetSRVHeap(),
		{
			L"../../Assets/Textures/wood_albedo.png" ,
			L"../../Assets/Textures/wood_normals.png" ,
			L"../../Assets/Textures/wood_roughness.png" ,
			L"../../Assets/Textures/wood_metal.png"
		},
		device,
		commandQueue,
		1 * MATERIAL_COUNT
	);

	cobblestoneMaterial = new Material(
		deferredRenderer->GetSRVHeap(),
		{
			L"../../Assets/Textures/rough_albedo.png" ,
			L"../../Assets/Textures/rough_normals.png" ,
			L"../../Assets/Textures/rough_roughness.png" ,
			L"../../Assets/Textures/rough_metal.png"
		},
		device,
		commandQueue,
		2 * MATERIAL_COUNT
	);

	deferredRenderer->SetIBLTextures(skyboxIRTexture, brdfLutTexture);

	deferredRenderer->SetSRV(skyboxTexture, DXGI_FORMAT_B8G8R8X8_UNORM, 3 * MATERIAL_COUNT, true);
	deferredRenderer->SetSRV(skyboxIRTexture, DXGI_FORMAT_B8G8R8X8_UNORM, 3 * MATERIAL_COUNT + 1, true);
	deferredRenderer->SetSRV(brdfLutTexture, DXGI_FORMAT_R8G8B8A8_UNORM, 3 * MATERIAL_COUNT + 2);

	entity1->SetMaterial(scratchedMaterial);
	entity2->SetMaterial(woodenMaterial);
	entity3->SetMaterial(cobblestoneMaterial);
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

void Game::Update()
{
	camera->Update(deltaTime);
	auto pos = XMFLOAT3(-2, 0, 2);
	entity1->SetPosition(pos);
	entity2->SetPosition(XMFLOAT3(2, 0, 2));
	entity3->SetPosition(XMFLOAT3(0, 0, 5));
	float rotX = 2 * sin(totalTime);
	entity1->SetRotation(XMFLOAT3(rotX, rotX, 0));
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
	deferredRenderer->Draw(commandList, { entity1, entity2, entity3 });

	deferredRenderer->SetLightShapePassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightShapePass(commandList, pixelCb);

	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	deferredRenderer->SetLightPassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightPass(commandList);

	deferredRenderer->DrawSkybox(commandList, rtvHandle, 3 * MATERIAL_COUNT);

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
	delete sphereMesh;
	delete cubeMesh;
	delete camera;
	delete entity1;
	delete entity2;
	delete entity3;
	delete scratchedMaterial;
	delete woodenMaterial;
	delete cobblestoneMaterial;

	skyboxTexture->Release();
	skyboxIRTexture->Release();
	brdfLutTexture->Release();

	//textureBuffer->Release();
	//normalTexture->Release();
	//roughnessTexture->Release();
	//metalnessTexture->Release();
}
