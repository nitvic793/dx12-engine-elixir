#pragma once

#include "Resources.h"

class ResourcePackSerDe
{
public:
	static Resources LoadResources(std::string filename);
	static void SaveResourcePack(std::string filename, Resources resourcePack);
	ResourcePackSerDe();
	~ResourcePackSerDe();
};

