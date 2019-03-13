#include "stdafx.h"
#include "Utility.h"

XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe)
{
	auto offset = *aiMe;
	XMFLOAT4X4 output;
	//auto mat = XMMatrixTranspose(XMMATRIX(&aiMe->a1));
	XMMATRIX mat = XMMatrixTranspose(
		XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
			offset.b1, offset.b2, offset.b3, offset.b4,
			offset.c1, offset.c2, offset.c3, offset.c4,
			offset.d1, offset.d2, offset.d3, offset.d4));
	XMStoreFloat4x4(&output, mat);
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
