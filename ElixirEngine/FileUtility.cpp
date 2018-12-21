#include "stdafx.h"
#include "FileUtility.h"

std::string GetFileNameWithoutExtension(std::string filePath)
{
	auto sIndex = filePath.find_last_of('/');
	auto dIndex = filePath.find_last_of('.');
	return filePath.substr(sIndex + 1, dIndex - sIndex - 1);
}
