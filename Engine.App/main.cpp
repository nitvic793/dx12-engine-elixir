#include "../ElixirEngine/Game.h"

//#include <vld.h>

HINSTANCE hinstLib;
BOOL fFreeResult;
typedef void(*LoadGameSystemsFunc)(std::vector<Elixir::ISystem*>& systems);
typedef void(*UnLoadGameSystemsFunc)(std::vector<Elixir::ISystem*>& systems);
LoadGameSystemsFunc LoadGameSystems;
UnLoadGameSystemsFunc UnloadGameSystems;

void LoadGameSystemsLibrary()
{
	// Get a handle to the DLL module.
	hinstLib = LoadLibrary(TEXT("Engine.Systems.dll"));    //1: load the DLL
	LoadGameSystems = (LoadGameSystemsFunc)GetProcAddress(hinstLib, "LoadSystems");
	UnloadGameSystems = (UnLoadGameSystemsFunc)GetProcAddress(hinstLib, "UnloadSystems");
}

void UnloadGameSystemsLibrary()
{
	// If the handle is valid, unload the DLL
	if (hinstLib != NULL)
	{
		fFreeResult = FreeLibrary(hinstLib);      //2: unload the DLL
	}
}

void BuildSystemsLibrary()
{
#ifdef _DEBUG
	system("msbuild ../../ElixirEngine.sln /target:Engine_Systems");
#else
	system("msbuild ../../ElixirEngine.sln /p:Configuration=Release /target:Engine_Systems");
#endif
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	LoadGameSystemsLibrary();
#if defined(DEBUG) | defined(_DEBUG)
	// Enable memory leak detection - quick and dirty
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// Ensure "Current Directory" (relative path) is always the .exe's folder
	{
		char currentDir[1024] = {};
		GetModuleFileName(0, currentDir, 1024);
		char* lastSlash = strrchr(currentDir, '\\');
		if (lastSlash)
		{
			*lastSlash = 0;
			SetCurrentDirectory(currentDir);
		}
	}

	Game* game = new Game(hInstance, nCmdShow, 1920, 1080, false);
	game->SetSystemsCallback([&](std::vector<Elixir::ISystem*>& systems)
	{
		UnloadGameSystems(systems);
		UnloadGameSystemsLibrary();
		BuildSystemsLibrary();
		LoadGameSystemsLibrary();
		LoadGameSystems(systems);
	}, 
	[&](std::vector<Elixir::ISystem*>& systems)
	{
		UnloadGameSystems(systems);
		UnloadGameSystemsLibrary();
	});

	game->Initialize();
	game->Run([&](Core* core)
	{
	});
	game->Shutdown();

	delete game;

	IDXGIDebug1* pDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
		pDebug->Release();
	}

	return 0;
}