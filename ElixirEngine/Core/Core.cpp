#include "Core.h"
#include "Vertex.h"
#include "ImageHelpers.h"
#include "ShaderManager.h"

Core* Core::coreInstance = nullptr;

LRESULT CALLBACK WindowsProcessMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Core::coreInstance->HandleWindowsCallback(hWnd, msg, wParam, lParam);
}

bool Core::InitD3D()
{
	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	IDXGIFactory4* dxgiFactory;
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return false;
	}

	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			adapterIndex++;
			continue;
		}

		//direct3d 12 (feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		return false;
	}

	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device)
	);

	if (FAILED(hr))
	{
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};

	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));
	if (FAILED(hr))
	{
		return false;
	}

	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = Width;
	backBufferDesc.Height = Height;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc = sampleDesc; // multi-sampling description
	swapChainDesc.Windowed = !Fullscreen; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(
		commandQueue,
		&swapChainDesc,
		&tempSwapChain
	);

	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr))
	{
		return false;
	}

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each buffer (double buffering is two buffers, triple buffering is 3).
	for (int i = 0; i < frameBufferCount; i++)
	{
		// get the n'th buffer in the swap chain and store it in the n'th
		// position of our ID3D12Resource array
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr))
		{
			return false;
		}
		device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

		//increment the rtv handle by the rtv descriptor size we got above
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr))
		{
			return false;
		}
	}

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		return false;
	}

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			return false;
		}
		fenceValue[i] = 0; // set initial fence value to 0
	}

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		return false;
	}

	return true;
}

void Core::InitResources()
{
	ResourceUploadBatch uploadBatch(device);
	deferredRenderer = new DeferredRenderer(device, Width, Height);
	deferredRenderer->Initialize(commandList);
	camera = new Camera((float)Width, (float)Height);
	entity1 = new Entity();
	entity2 = new Entity();
	entity3 = new Entity();

	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
	rootCBVDescriptor.RegisterSpace = 0;
	rootCBVDescriptor.ShaderRegister = 0;

	D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of shader resource views (descriptors)
	descriptorTableRanges[0].NumDescriptors = 1; // we only have one texture right now, so the range is only 1
	descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
	descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

	D3D12_ROOT_PARAMETER  rootParameters[3];
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[0].Descriptor = rootCBVDescriptor;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // this is a constant buffer view root descriptor
	rootParameters[1].Descriptor = rootCBVDescriptor;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
	rootParameters[2].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // our pixel shader will be the only shader accessing this parameter for now

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_ANISOTROPIC;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 16;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), // we have 2 root parameters
		rootParameters, // a pointer to the beginning of our root parameters array
		1, // we have one static sampler
		&sampler, // a pointer to our static sampler (array)
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* signature;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	auto vertexShaderBytecode = ShaderManager::LoadShader(L"DefaultVS.cso");
	auto pixelShaderBytecode = ShaderManager::LoadShader(L"DefaultPS.cso");

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
	psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
	psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
	psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
	psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
	psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
	psoDesc.NumRenderTargets = 1; // we are only binding one render target
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));

	mesh = new Mesh("../../Assets/sphere.obj", device, commandList);

	entity1->SetMesh(mesh);
	entity2->SetMesh(mesh);
	entity3->SetMesh(mesh);

	//Create depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	);

	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	pixelCb.light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 0);
	pixelCb.light.DiffuseColor = XMFLOAT4(1.f, 0.0f, 0.f, 0.f);
	pixelCb.light.Direction = XMFLOAT3(1.f, 0.f, 0.f);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));

	uploadBatch.Begin();
	CreateWICTextureFromFile(device, uploadBatch, L"../../Assets/metal.jpg", &textureBuffer, false);
	CreateWICTextureFromFile(device, uploadBatch, L"../../Assets/metalNormal.png", &normalTexture, true);
	auto uploadOperation = uploadBatch.End(commandQueue);
	uploadOperation.wait();

	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	deferredRenderer->SetSRV(textureBuffer, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	deferredRenderer->SetSRV(normalTexture, DXGI_FORMAT_B8G8R8A8_UNORM, 1);
	//device->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Now we execute the command list to upload the initial assets (triangle data)
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };

	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


	//deferredRenderer->SetSRV(normalTexture, textureDesc.Format, 1);
	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue[frameIndex]++;
	auto hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
	}

	// Fill out the Viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)Width;
	viewport.Height = (float)Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Width;
	scissorRect.bottom = Height;

	pixelCb.pointLight = PointLight{ {0.f, 1.f, 0.f, 0.f} , {0.5f, 0.f, 1.f}, 6.f };
}

void Core::Update()
{
	camera->Update(deltaTime);
	auto pos = XMFLOAT3(-3, 0, 2);
	entity1->SetPosition(pos);
	entity2->SetPosition(XMFLOAT3(3, 0, 2));
	entity3->SetPosition(XMFLOAT3(0.5, 1, 1));
}

void Core::UpdatePipeline()
{
	HRESULT hr;

	//wait for the gpu to finish with the command allocator before we reset it
	WaitForPreviousFrame();

	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr))
	{
		Running = false;
	}
	hr = commandList->Reset(commandAllocator[frameIndex], pipelineStateObject);
	if (FAILED(hr))
	{
		Running = false;
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pixelCb.invProjView = camera->GetInverseProjectionViewMatrix();

	// draw
	deferredRenderer->SetGBUfferPSO(commandList, { entity1, entity2, entity3 }, camera, pixelCb);
	deferredRenderer->Draw(commandList);

	deferredRenderer->SetLightShapePassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightShapePass(commandList, pixelCb);

	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	
	deferredRenderer->SetLightPassPSO(commandList, pixelCb);
	deferredRenderer->DrawLightPass(commandList);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = false;
	}
}

void Core::Render()
{
	HRESULT hr;

	UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

	ID3D12CommandList* ppCommandLists[] = { commandList };

	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
	}
	hr = swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		HRESULT r = device->GetDeviceRemovedReason();
		Running = false;
	}
}

void Core::Cleanup()
{
	// wait for the gpu to finish all frames
	for (int i = 0; i < frameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}

	// get swapchain out of full screen before exiting
	BOOL fs = false;
	if (swapChain->GetFullscreenState(&fs, NULL))
		swapChain->SetFullscreenState(false, NULL);

	if (device)device->Release();
	swapChain->Release();
	commandQueue->Release();
	rtvDescriptorHeap->Release();
	commandList->Release();

	for (int i = 0; i < frameBufferCount; ++i)
	{
		renderTargets[i]->Release();
		commandAllocator[i]->Release();
		fence[i]->Release();
	};

	mainDescriptorHeap->Release();
	pipelineStateObject->Release();
	rootSignature->Release();
	//vertexBuffer->Release();
	//indexBuffer->Release();
	depthStencilBuffer->Release();
	dsDescriptorHeap->Release();
	textureBuffer->Release();
	//textureBufferUploadHeap->Release();

	delete mesh;
	delete camera;
	delete entity1;
	delete entity2;
	delete entity3;
	delete deferredRenderer;
}

void Core::WaitForPreviousFrame()
{
	HRESULT hr;
	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
}

void Core::UpdateTimer()
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	currentTime = now;

	deltaTime = max((float)((currentTime - previousTime) * perfCounterSeconds), 0.0f);
	totalTime = (float)((currentTime - startTime) * perfCounterSeconds);
	previousTime = currentTime;
}

Core::Core(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen)
{
	coreInstance = this;
	Running = true;
	this->InitializeWindow(hInstance, ShowWnd, width, height, fullscreen);
	this->InitD3D();
	this->InitResources();

	fpsFrameCount = 0;
	fpsTimeElapsed = 0.0f;

	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	perfCounterSeconds = 1.0 / (double)perfFreq;
}

void Core::InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen)
{
	Width = width;
	Fullscreen = fullscreen;
	Height = height;

	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowsProcessMessage;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpszClassName = WindowName.c_str();

	RegisterClassEx(&wndClass);
	RECT clientRect;
	SetRect(&clientRect, 0, 0, width, height);
	AdjustWindowRect(
		&clientRect,
		WS_OVERLAPPEDWINDOW,	// Has a title bar, border, min and max buttons, etc.
		false);

	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

	hwnd = CreateWindow(
		wndClass.lpszClassName,
		WindowTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		centeredX,
		centeredY,
		clientRect.right - clientRect.left,	// Calculated width
		clientRect.bottom - clientRect.top,	// Calculated height
		0,			// No parent window
		0,			// No menu
		hInstance,	// The app's handle
		0);	  // used with multiple windows, NULL

	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);
}

void Core::Run(std::function<void(Core*)> coreLogicCallback)
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	startTime = now;
	currentTime = now;
	previousTime = now;

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UpdateTimer();
			if (coreLogicCallback)
			{
				coreLogicCallback(this);
			}

			Update();
			Render();
		}
	}

	WaitForPreviousFrame();
	CloseHandle(fenceEvent); // close the fence event
}

LRESULT Core::HandleWindowsCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, "Are you sure you want to exit?",
				"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}



Core::~Core()
{
	Cleanup();
}

