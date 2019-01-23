#include "stdafx.h"
#include "StringHash.h"

unsigned int GenerateHash(const char * string, size_t len)
{
	unsigned int hash = 0;
	for (size_t i = 0; i < len; ++i)
		hash = 65599 * hash + string[i];
	return hash ^ (hash >> 16);
}


std::wstring ToWideString(const std::string& s)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//std::string narrow = converter.to_bytes(wide_utf16_source_string);
	std::wstring wide = converter.from_bytes(s.c_str());
	return wide;
}