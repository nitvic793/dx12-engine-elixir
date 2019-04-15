#pragma once
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/Importer.hpp>      // C++ importer interface
#include "OGLMath.h"
#include "Core/Camera.h"

XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe);
XMFLOAT3X3 aiMatrixToXMFloat3x3(const aiMatrix3x3* aiMe);

XMMATRIX OGLtoXM(const ogldev::Matrix4f& mat);

bool IsIntersecting(DirectX::BoundingOrientedBox boundingBox, Camera* camera, int mouseX, int mouseY, float& distance);