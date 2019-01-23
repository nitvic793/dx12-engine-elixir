#include "stdafx.h"
#include "ResourcePackSerDe.h"
#include <fstream>
#include <cereal/archives/json.hpp>

Resources ResourcePackSerDe::LoadResources(std::string filename)
{
	Resources pack;
	std::ifstream is(filename.c_str());
	cereal::JSONInputArchive archive(is);
	archive(cereal::make_nvp("Resources", pack));
	return pack;
}

void ResourcePackSerDe::SaveResourcePack(std::string filename, Resources resourcePack)
{
	std::ofstream os(filename.c_str());
	cereal::JSONOutputArchive archive(os);
	archive(cereal::make_nvp("Resources", resourcePack));
}

ResourcePackSerDe::ResourcePackSerDe()
{
}


ResourcePackSerDe::~ResourcePackSerDe()
{
}
