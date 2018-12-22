#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>
#include <wincodec.h>
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "DirectXTex.h"
#include "ResourceUploadBatch.h"
#include "DirectXHelpers.h"
#include <dxgidebug.h>
#include <DirectXCollision.h>
#include "FileUtility.h"

using namespace DirectX;

#ifndef FRAMEBUFFERCOUNT
#define FRAMEBUFFERCOUNT 3
#endif 

