#pragma once

#include "stdafx.h"

struct MaterialType
{
	HashID MaterialID;
	std::string AlbedoPath;
	std::string NormalPath;
	std::string RoughnessPath;
	std::string MetalnessPath;
};

struct MaterialData
{
	std::vector<MaterialType> Materials;
};