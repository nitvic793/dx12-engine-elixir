#include "stdafx.h"
#include "MathHelper.h"

XMFLOAT4X4 Elixir::GetWorldViewProjectionTransposed(const XMFLOAT4X4 & world, const XMFLOAT4X4 & view, const XMFLOAT4X4 & projection)
{
	XMMATRIX viewMat = XMLoadFloat4x4(&view); // load view matrix
	XMMATRIX projMat = XMLoadFloat4x4(&projection); // load projection matrix
	XMMATRIX wvpMat = XMLoadFloat4x4(&world) * viewMat * projMat; // create wvp matrix
	XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	XMFLOAT4X4 wvp;
	XMStoreFloat4x4(&wvp, transposed); // store transposed wvp matrix in constant buffer
	return wvp;
}

XMFLOAT4X4 Elixir::Transpose(const XMFLOAT4X4 & matrix)
{
	auto mat = XMLoadFloat4x4(&matrix);
	XMFLOAT4X4 matT;
	XMStoreFloat4x4(&matT, XMMatrixTranspose(mat));
	return matT;
}
