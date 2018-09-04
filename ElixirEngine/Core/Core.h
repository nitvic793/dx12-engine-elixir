#include "../stdafx.h"
#include <string>
#include <functional>

#ifndef FRAMEBUFFERCOUNT
#define FRAMEBUFFERCOUNT 3
#endif 


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

	ID3D12Fence* fence[FRAMEBUFFERCOUNT];    // an object that is locked while command list is being executed by the gpu.
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
