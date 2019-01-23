#include "stdafx.h"
#include "SceneSerDe.h"
#include <fstream>
#include <cereal/archives/json.hpp>

void SceneSerDe::SaveScene(Scene scene, std::string filename)
{
	std::ofstream os(filename.c_str());
	cereal::JSONOutputArchive archive(os);
	archive(cereal::make_nvp("Scene", scene));
}

Scene SceneSerDe::LoadScene(std::string filename)
{
	Scene outScene;
	std::ifstream is(filename.c_str());
	cereal::JSONInputArchive archive(is);
	archive(cereal::make_nvp("Scene", outScene));
	return outScene;
}

SceneSerDe::SceneSerDe()
{
}


SceneSerDe::~SceneSerDe()
{
}
