#include "stdafx.h"
#include "Core/Core.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
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

	Core* core = new Core(hInstance, nCmdShow, 1280, 720, false);
	core->Run([&](Core* core) 
	{
		//Game Logic here...
	});
	return 0;
}