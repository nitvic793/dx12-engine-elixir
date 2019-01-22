#pragma once

#include "stdafx.h"

class MeshType
{
	HashID MeshID;
	std::string MeshPath;
};

class MeshData
{
	std::vector<MeshType> Meshes;
};