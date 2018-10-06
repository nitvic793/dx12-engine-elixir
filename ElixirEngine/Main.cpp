#include "stdafx.h"
#include "Game.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
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

	Game* game = new Game(hInstance, nCmdShow, 1280, 720, false);
	game->Initialize();
	game->Run([&](Core* core) 
	{
		//Game Logic here...
	});

	delete game;
	return 0;
}