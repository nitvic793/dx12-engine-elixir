#include "stdafx.h"
#include "StringHash.h"

unsigned int GenerateHash(const char * string, size_t len)
{
	unsigned int hash = 0;
	for (size_t i = 0; i < len; ++i)
		hash = 65599 * hash + string[i];
	return hash ^ (hash >> 16);
}
