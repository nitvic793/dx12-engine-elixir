#pragma once

unsigned int GenerateHash(const char *string, size_t len);

//Credits: http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash
#ifndef HASHF

#define HASHF
#define H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<strlen(s)?strlen(s)-1-(i):strlen(s)])
#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
#define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))

//Generates hash for given string
#define SID(s)    ((uint32_t)(H256(s,0,0)^(H256(s,0,0)>>16)))

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

	operator HashID()
	{
		return hash;
	}

	HashID GetHash()
	{
		return hash;
	}
};