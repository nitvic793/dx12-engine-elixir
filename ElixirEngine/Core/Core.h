#include "../stdafx.h"
#include "ConstantBuffer.h"
#include <string>
#include <functional>

class Core
{

public:
	static Core* coreInstance;
	Core(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);
	void Run(std::function<void(Core*)> coreLogicCallback);
	LRESULT HandleWindowsCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	~Core();

protected:
	HWND hwnd = NULL;
	std::string WindowName = "Elixir Engine";
	std::string WindowTitle = "Elixir Game";

	int Width;
	int Height;
	bool Fullscreen;
	bool Running;

	//Direct3D 

	const int frameBufferCount = FRAMEBUFFERCOUNT; 

	ID3D12Device* device; 
	IDXGISwapChain3* swapChain; 
	ID3D12DescriptorHeap* rtvDescriptorHeap; 
	ID3D12Resource* renderTargets[FRAMEBUFFERCOUNT]; 

	ID3D12CommandQueue* commandQueue;
	ID3D12CommandAllocator* commandAllocator[FRAMEBUFFERCOUNT]; 
	ID3D12GraphicsCommandList* commandList;

	ID3D12Fence* fence[FRAMEBUFFERCOUNT];  // an object that is locked while command list is being executed by the gpu.
	HANDLE fenceEvent; // a handle to an event when fence is unlocked by the gpu
	UINT64 fenceValue[FRAMEBUFFERCOUNT]; // this value is incremented each frame. each fence will have its own value

	ID3D12PipelineState* pipelineStateObject; 
	ID3D12RootSignature* rootSignature; // root signature defines data shaders will access
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto
	ID3D12Resource* vertexBuffer; 
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ID3D12Resource* indexBuffer; 
	D3D12_INDEX_BUFFER_VIEW indexBufferView; 
	ID3D12Resource* depthStencilBuffer; 
	ID3D12DescriptorHeap* dsDescriptorHeap; // heap for depth stencil buffer descriptor

	ID3D12DescriptorHeap* mainDescriptorHeap[FRAMEBUFFERCOUNT]; // this heap will store the descripor to our constant buffer
	ID3D12Resource* constantBufferUploadHeap[FRAMEBUFFERCOUNT]; // this is the memory on the gpu where our constant buffer will be placed.

	ConstantBuffer cbPerObject; // this is the constant buffer data we will send to the gpu 

	UINT8* cbGPUAddress[FRAMEBUFFERCOUNT]; // this is a pointer to the memory location we get when we map our constant buffer

	int frameIndex; // current rtv
	int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

	bool InitD3D();
	void InitResources();
	void Update(); // update the game logic
	void UpdatePipeline(); 
	void Render(); 
	void Cleanup();
	void WaitForPreviousFrame(); // wait until gpu is finished with command list

	void InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);
};
