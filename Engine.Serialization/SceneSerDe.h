#pragma once

#include "Scene.h"

class SceneSerDe
{
public:
	static void		SaveScene(Scene scene, std::string filename);
	static Scene	LoadScene(std::string filename);
	SceneSerDe();
	~SceneSerDe();
};

