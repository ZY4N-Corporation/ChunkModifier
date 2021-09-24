#pragma once

#include <bitset>
#include <stdexcept>
#include <string>
#include <vector>

#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <texture_fetch_functions.h>
#include <texture_indirect_functions.h>

#include <Image.hpp>
#include <cStructs.h>
#include <color.h>

#define checkCUDA(ans) { checkCUDAerror((ans), __FILE__, __LINE__); }

inline void checkCUDAerror(const cudaError status, const char* file, int line, bool abort = true) {
	if (status != cudaSuccess) {
		throw std::runtime_error(std::string(cudaGetErrorString(status)) + " " + std::string(file) + " " + std::to_string(line));
	}
}

namespace CUDA {
	void insertBlocks(size_t numBlocks, size_t numThreads, size_t* indexBuffer, size_t numIndices, cudaTextureObject_t tex, uint16_t* blockBuffer, int chunkX, int chunkZ);
	
	void setMinChunkY(size_t v);
	void setLookupSize(size_t v);

	cudaTextureObject_t* createTexture(Image&);

	void initVertexBuffer(std::vector<Vec3> &buffer);
	void initTexCoordBuffer(std::vector<Vec2> &buffer);
	void initTriangleBuffer(std::vector<Triangle> &buffer);
	void initLookupColorBuffer(std::vector<color> &buffer);
	void initLookupIndexBuffer(std::vector<uint16_t> &buffer);

	void freeVertexBuffer();
	void freeTexCoordBuffer();
	void freeTriangleBuffer();
	void freeLookupColorBuffer();
	void freeLookupIndexBuffer();
}
