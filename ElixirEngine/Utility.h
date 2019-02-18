#pragma once
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/Importer.hpp>      // C++ importer interface

XMFLOAT4X4 aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe);