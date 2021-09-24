#pragma once

#include <bitset>

typedef struct {
	float x, y, z;
} Vec3;

typedef struct {
	float u, v;
} Vec2;

typedef struct {
	size_t vertexIndices[3];
	size_t texCoordIndices[3];
	uint16_t blockID;
} Triangle;
