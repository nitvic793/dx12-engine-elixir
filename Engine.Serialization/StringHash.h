#pragma once

#include <string>
#include <locale>
#include <codecvt>
#include <string>


unsigned int GenerateHash(const char *string, size_t len);

std::wstring ToWideString(const std::string& s);


//Credits: http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash
#ifndef HASHF

#define HASHF
#define Hash1(s,i,x)   (x*65599u+(uint8_t)s[(i)<strlen(s)?strlen(s)-1-(i):strlen(s)])
#define Hash4(s,i,x)   Hash1(s,i,Hash1(s,i+1,Hash1(s,i+2,Hash1(s,i+3,x))))
#define Hash16(s,i,x)  Hash4(s,i,Hash4(s,i+4,Hash4(s,i+8,Hash4(s,i+12,x))))
#define Hash64(s,i,x)  Hash16(s,i,Hash16(s,i+16,Hash16(s,i+32,Hash16(s,i+48,x))))
#define Hash256(s,i,x) Hash64(s,i,Hash64(s,i+64,Hash64(s,i+128,Hash64(s,i+192,x))))

//Generates hash for given string
#define SID(s)    ((uint32_t)(Hash256(s,0,0)^(Hash256(s,0,0)>>16)))

#endif // HASHF

typedef unsigned int HashID;

class StringID
{
	HashID hash;
public:
	StringID(unsigned int hashValue) :
		hash(hashValue)
	{
	}

	StringID(const char* stringValue) :
		hash(SID(stringValue))
		//hash(GenerateHash(stringValue, strlen(stringValue)))
	{

	}

	StringID(std::string stringValue) :
		hash(SID(stringValue.c_str()))
		//hash(GenerateHash(stringValue, strlen(stringValue)))
	{

	}

	operator HashID()
	{
		return hash;
	}

	HashID GetHash()
	{
		return hash;
	}
};

