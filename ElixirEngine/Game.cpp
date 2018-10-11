#include "stdafx.h"
#include "Game.h"

//Initializes assets. This function's scope has access to commandList which is not closed. 
void Game::InitializeAssets()
{
	camera = new Camera((float)Width, (float)Height);
	entity1 = new Entity();
	entity2 = new Entity();
	entity3 = new Entity();
	mesh = new Mesh("../../Assets/sphere.obj", device, commandList);

	entity1->SetMesh(mesh);
	entity2->SetMesh(mesh);
	entity3->SetMesh(mesh);

	pixelCb.light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light.DiffuseColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 0.f);
	pixelCb.light.Direction = XMFLOAT3(0.f, 0.f, 1.f);
	pixelCb.pointLight = PointLight{ {0.2f, 0.6f, 0.2f, 0.f} , {0.0f, 1.f, 1.f}, 10.f };

	ResourceUploadBatch uploadBatch(device);
	uploadBatch.Begin();
	CreateWICTextureFromFile(device, uploadBatch, L"../../Assets/Textures/scratched_albedo.png", &textureBuffer, false);
	CreateWICTextureFromFile(device, uploadBatch, L"../../Assets/Textures/scratched_normals.png", &normalTexture, true);
	CreateWICTextureFromFile(device, uploadBatch, L"../../Assets/Textures/scratched_roughness.png", &roughnessTexture, true);
	CreateWICTextureFromFile(device, uploadBatch, L"../../Assets/Textures/scratched_metal.png", &metalnessTexture, true);
	auto uploadOperation = uploadBatch.End(commandQueue);
	uploadOperation.wait();

	/*CreateShaderResourceView(device, textureBuffer, deferredRenderer->GetSRVHeap().handleCPU(0));
	CreateShaderResourceView(device, normalTexture, deferredRenderer->GetSRVHeap().handleCPU(1));
	CreateShaderResourceView(device, roughnessTexture, deferredRenderer->GetSRVHeap().handleCPU(2));
	CreateShaderResourceView(device, metalnessTexture, deferredRenderer->GetSRVHeap().handleCPU(3));*/

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

	entity1->SetMaterial(scratchedMaterial);
	entity2->SetMaterial(woodenMaterial);
	entity3->SetMaterial(cobblestoneMaterial);

	//deferredRenderer->SetSRV(textureBuffer, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	//deferredRenderer->SetSRV(normalTexture, DXGI_FORMAT_B8G8R8A8_UNORM, 1);
	//deferredRenderer->SetSRV(roughnessTexture, DXGI_FORMAT_R8G8B8A8_UNORM, 2);
	//deferredRenderer->SetSRV(metalnessTexture, DXGI_FORMAT_R8G8B8A8_UNORM, 3);
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
	delete mesh;
	delete camera;
	delete entity1;
	delete entity2;
	delete entity3;
	delete scratchedMaterial;
	delete woodenMaterial;
	delete cobblestoneMaterial;

	textureBuffer->Release();
	normalTexture->Release();
	//roughnessTexture->Release();
	//metalnessTexture->Release();
}
