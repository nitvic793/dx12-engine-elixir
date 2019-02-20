#include "stdafx.h"
#include "Utility.h"

XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe)
{
	XMFLOAT4X4 output;
	output._11 = aiMe->a1;
	output._12 = aiMe->a2;
	output._13 = aiMe->a3;
	output._14 = aiMe->a4;

	output._21 = aiMe->b1;
	output._22 = aiMe->b2;
	output._23 = aiMe->b3;
	output._24 = aiMe->b4;

	output._31 = aiMe->c1;
	output._32 = aiMe->c2;
	output._33 = aiMe->c3;
	output._34 = aiMe->c4;

	output._41 = aiMe->d1;
	output._42 = aiMe->d2;
	output._43 = aiMe->d3;
	output._44 = aiMe->d4;

	return output;
}

XMFLOAT3X3 aiMatrixToXMFloat3x3(const aiMatrix3x3 * aiMe)
{
	XMFLOAT3X3 output;
	output._11 = aiMe->a1;
	output._12 = aiMe->a2;
	output._13 = aiMe->a3;

	output._21 = aiMe->b1;
	output._22 = aiMe->b2;
	output._23 = aiMe->b3;

	output._31 = aiMe->c1;
	output._32 = aiMe->c2;
	output._33 = aiMe->c3;

	return output;
}
