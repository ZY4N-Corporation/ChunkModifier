#include "kernel.cuh"

__constant__ size_t CUDA_minChunkY;
__constant__ size_t CUDA_lookupSize;

__device__ Vec3* CUDA_vertices;
__device__ Vec2* CUDA_texCoords;
__device__ Triangle* CUDA_triangles;

__device__ color* CUDA_lookupColors;
__device__ uint16_t* CUDA_lookupIndices;

namespace CUDA {

	void setMinChunkY(size_t chunkY) {
		checkCUDA(cudaMemcpyToSymbol(CUDA_minChunkY, (const void*)&chunkY, sizeof(size_t), 0, cudaMemcpyHostToDevice));
	}

	void setLookupSize(size_t size) {
		checkCUDA(cudaMemcpyToSymbol(CUDA_lookupSize, (const void*)&size, sizeof(size_t), 0, cudaMemcpyHostToDevice));
	}

	cudaTextureObject_t* createTexture(Image& img) {

		cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(8, 8, 8, 8, cudaChannelFormatKindUnsigned);
		cudaArray_t cuArray;
		cudaMallocArray(&cuArray, &channelDesc, img.width, img.height);

		const size_t spitch = (size_t)img.width * img.numChannels * sizeof(uint8_t);

		size_t texAtlasSize = 0;
		const uint8_t* pixels = img.pixels(texAtlasSize);
		cudaMemcpy2DToArray(cuArray, 0, 0, pixels, spitch, spitch, img.height, cudaMemcpyHostToDevice);

		cudaResourceDesc resDesc{ cudaResourceTypeArray, { cuArray } };

		cudaTextureDesc texDesc;
		memset(&texDesc, 0, sizeof(texDesc));
		texDesc.addressMode[0] = cudaAddressModeClamp;
		texDesc.addressMode[1] = cudaAddressModeClamp;
		texDesc.filterMode = cudaFilterModePoint;
		texDesc.readMode = cudaReadModeElementType;
		texDesc.normalizedCoords = 1;

		cudaTextureObject_t* texture = new cudaTextureObject_t{};

		cudaCreateTextureObject(texture, &resDesc, &texDesc, NULL);

		return texture;
	}

	//-----------------/ buffer init /-----------------//

	void initVertexBuffer(std::vector<Vec3> &buffer) {	
		Vec3* tmp_devicePtr;
		cudaMalloc(&tmp_devicePtr, buffer.size() * sizeof(Vec3));
		cudaMemcpy(tmp_devicePtr, buffer.data(), buffer.size() * sizeof(Vec3), cudaMemcpyHostToDevice);
		cudaMemcpyToSymbol(CUDA_vertices, &tmp_devicePtr, sizeof(Vec3*));
	}

	void initTexCoordBuffer(std::vector<Vec2> &buffer) {
		Vec3* tmp_devicePtr;
		cudaMalloc(&tmp_devicePtr, buffer.size() * sizeof(Vec2));
		cudaMemcpy(tmp_devicePtr, buffer.data(), buffer.size() * sizeof(Vec2), cudaMemcpyHostToDevice);
		cudaMemcpyToSymbol(CUDA_texCoords, &tmp_devicePtr, sizeof(Vec2*));
	}

	void initTriangleBuffer(std::vector<Triangle> &buffer) {
		Vec3* tmp_devicePtr;
		cudaMalloc(&tmp_devicePtr, buffer.size() * sizeof(Triangle));
		cudaMemcpy(tmp_devicePtr, buffer.data(), buffer.size() * sizeof(Triangle), cudaMemcpyHostToDevice);
		cudaMemcpyToSymbol(CUDA_triangles, &tmp_devicePtr, sizeof(Triangle*));
	}

	void initLookupColorBuffer(std::vector<color> &buffer) {
		uint8_t* tmp_devicePtr;
		cudaMalloc(&tmp_devicePtr, buffer.size() * sizeof(uint8_t));
		cudaMemcpy(tmp_devicePtr, buffer.data(), buffer.size() * sizeof(uint8_t), cudaMemcpyHostToDevice);
		cudaMemcpyToSymbol(CUDA_lookupColors, &tmp_devicePtr, sizeof(uint8_t*));
	}

	void initLookupIndexBuffer(std::vector<uint16_t> &buffer) {
		uint16_t* tmp_devicePtr;
		cudaMalloc(&tmp_devicePtr, buffer.size() * sizeof(uint16_t));
		cudaMemcpy(tmp_devicePtr, buffer.data(), buffer.size() * sizeof(uint16_t), cudaMemcpyHostToDevice);
		cudaMemcpyToSymbol(CUDA_lookupIndices, &tmp_devicePtr, sizeof(uint16_t*));
	}

	//-----------------/ buffer cleanup /-----------------//

	void freeVertexBuffer() {
		checkCUDA(cudaFree(CUDA_vertices));
	}

	void freeTexCoordBuffer() {
		checkCUDA(cudaFree(CUDA_texCoords));
	}

	void freeTriangleBuffer() {
		checkCUDA(cudaFree(CUDA_triangles));
	}


	void freeLookupColorBuffer() {
		checkCUDA(cudaFree(CUDA_lookupColors));
	}

	void freeLookupIndexBuffer() {
		checkCUDA(cudaFree(CUDA_lookupIndices));
	}

}

//-----------------/ vector math /-----------------//

__device__ Vec3 add(const Vec3& p1, const Vec3& p2) {
	return {
		p1.x + p2.x,
		p1.y + p2.y,
		p1.z + p2.z
	};
}

__device__ Vec3 sub(const Vec3& p1, const Vec3& p2) {
	return {
		p1.x - p2.x,
		p1.y - p2.y,
		p1.z - p2.z
	};
}

__device__ Vec3 cross(const Vec3& p1, const Vec3& p2) {
	return {
		p1.y * p2.z - p2.y * p1.z,
		p1.z * p2.x - p2.z * p1.x,
		p1.x * p2.y - p2.x * p1.y
	};
}

__device__ float dot(const Vec3& p1, const Vec3& p2) {
	return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}

//-----------------/ AABB triangle collision /-----------------//

__device__ bool cuda_approxTriBoxOverlap(const Vec3& min, const Vec3& max, const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	return !((v0.x > max.x && v1.x > max.x && v2.x > max.x) ||
		(v0.y > max.y && v1.y > max.y && v2.y > max.y) ||
		(v0.z > max.z && v1.z > max.z && v2.z > max.z) ||
		(v0.x < min.x&& v1.x < min.x&& v2.x < min.x) ||
		(v0.y < min.y&& v1.y < min.y&& v2.y < min.y) ||
		(v0.z < min.z&& v1.z < min.z&& v2.z < min.z));
}

__device__ void cuda_findMinMax(float x0, float x1, float x2, float& min, float& max) {
	min = max = x0;
	if (x1 < min)
		min = x1;
	if (x1 > max)
		max = x1;
	if (x2 < min)
		min = x2;
	if (x2 > max)
		max = x2;
}

__device__ bool cuda_planeBoxOverlap(const Vec3& normal, const Vec3& vert, const Vec3& maxbox) {
	Vec3 vmin{ 0, 0, 0 };
	Vec3 vmax{ 0, 0, 0 };

	if (normal.x > 0.0f) {
		vmin.x = -maxbox.x - vert.x;
		vmax.x = maxbox.x - vert.x;
	} else {
		vmin.x = maxbox.x - vert.x;
		vmax.x = -maxbox.x - vert.x;
	}

	if (normal.y > 0.0f) {
		vmin.y = -maxbox.y - vert.y;
		vmax.y = maxbox.y - vert.y;
	} else {
		vmin.y = maxbox.y - vert.y;
		vmax.y = -maxbox.y - vert.y;
	}

	if (normal.z > 0.0f) {
		vmin.z = -maxbox.z - vert.z;
		vmax.z = maxbox.z - vert.z;
	} else {
		vmin.z = maxbox.z - vert.z;
		vmax.z = -maxbox.z - vert.z;
	}

	if (dot(normal, vmin) > 0.0f)
		return false;

	if (dot(normal, vmax) >= 0.0f)
		return true;

	return false;
}

__device__ bool cuda_axisTestX01(float a, float b, float fa, float fb, const Vec3& v0,
	const Vec3& v2, const Vec3& boxhalfsize, float& rad, float& min,
	float& max, float& p0, float& p2) {
	p0 = a * v0.y - b * v0.z;
	p2 = a * v2.y - b * v2.z;
	if (p0 < p2) {
		min = p0;
		max = p2;
	} else {
		min = p2;
		max = p0;
	}
	rad = fa * boxhalfsize.y + fb * boxhalfsize.z;
	if (min > rad || max < -rad)
		return false;
	return true;
}

__device__ bool cuda_axisTestX2(float a, float b, float fa, float fb, const Vec3& v0,
	const Vec3& v1, const Vec3& boxhalfsize, float& rad, float& min,
	float& max, float& p0, float& p1) {
	p0 = a * v0.y - b * v0.z;
	p1 = a * v1.y - b * v1.z;
	if (p0 < p1) {
		min = p0;
		max = p1;
	} else {
		min = p1;
		max = p0;
	}
	rad = fa * boxhalfsize.y + fb * boxhalfsize.z;
	if (min > rad || max < -rad)
		return false;
	return true;
}

__device__ bool cuda_axisTestY02(float a, float b, float fa, float fb, const Vec3& v0,
	const Vec3& v2, const Vec3& boxhalfsize, float& rad, float& min,
	float& max, float& p0, float& p2) {
	p0 = -a * v0.x + b * v0.z;
	p2 = -a * v2.x + b * v2.z;
	if (p0 < p2) {
		min = p0;
		max = p2;
	} else {
		min = p2;
		max = p0;
	}
	rad = fa * boxhalfsize.x + fb * boxhalfsize.z;
	if (min > rad || max < -rad)
		return false;
	return true;
}

__device__ bool cuda_axisTestY1(float a, float b, float fa, float fb, const Vec3& v0,
	const Vec3& v1, const Vec3& boxhalfsize, float& rad, float& min,
	float& max, float& p0, float& p1) {
	p0 = -a * v0.x + b * v0.z;
	p1 = -a * v1.x + b * v1.z;
	if (p0 < p1) {
		min = p0;
		max = p1;
	} else {
		min = p1;
		max = p0;
	}
	rad = fa * boxhalfsize.x + fb * boxhalfsize.z;
	if (min > rad || max < -rad)
		return false;
	return true;
}

__device__ bool cuda_axisTestZ12(float a, float b, float fa, float fb, const Vec3& v1,
	const Vec3& v2, const Vec3& boxhalfsize, float& rad, float& min,
	float& max, float& p1, float& p2) {
	p1 = a * v1.x - b * v1.y;
	p2 = a * v2.x - b * v2.y;
	if (p1 < p2) {
		min = p1;
		max = p2;
	} else {
		min = p2;
		max = p1;
	}
	rad = fa * boxhalfsize.x + fb * boxhalfsize.y;
	if (min > rad || max < -rad)
		return false;
	return true;
}

__device__ bool cuda_axisTestZ0(float a, float b, float fa, float fb, const Vec3& v0,
	const Vec3& v1, const Vec3& boxhalfsize, float& rad, float& min,
	float& max, float& p0, float& p1) {
	p0 = a * v0.x - b * v0.y;
	p1 = a * v1.x - b * v1.y;
	if (p0 < p1) {
		min = p0;
		max = p1;
	} else {
		min = p1;
		max = p0;
	}
	rad = fa * boxhalfsize.x + fb * boxhalfsize.y;
	if (min > rad || max < -rad)
		return false;
	return true;
}

__device__ bool cuda_triBoxOverlap(const Vec3& boxcenter, const Vec3& boxhalfsize, const Vec3& tv0, const Vec3& tv1, const Vec3& tv2) {

	Vec3 v0, v1, v2;
	float min, max, p0, p1, p2, rad, fex, fey, fez;
	Vec3 normal, e0, e1, e2;

	v0 = sub(tv0, boxcenter);
	v1 = sub(tv1, boxcenter);
	v2 = sub(tv2, boxcenter);

	e0 = sub(v1, v0);
	e1 = sub(v2, v1);
	e2 = sub(v0, v2);

	fex = fabsf(e0.x);
	fey = fabsf(e0.y);
	fez = fabsf(e0.z);

	if (!cuda_axisTestX01(e0.z, e0.y, fez, fey, v0, v2, boxhalfsize, rad, min, max, p0, p2))
		return false;
	if (!cuda_axisTestY02(e0.z, e0.x, fez, fex, v0, v2, boxhalfsize, rad, min, max, p0, p2))
		return false;
	if (!cuda_axisTestZ12(e0.y, e0.x, fey, fex, v1, v2, boxhalfsize, rad, min, max, p1, p2))
		return false;

	fex = fabsf(e1.x);
	fey = fabsf(e1.y);
	fez = fabsf(e1.z);

	if (!cuda_axisTestX01(e1.z, e1.y, fez, fey, v0, v2, boxhalfsize, rad, min, max, p0, p2))
		return false;
	if (!cuda_axisTestY02(e1.z, e1.x, fez, fex, v0, v2, boxhalfsize, rad, min, max, p0, p2))
		return false;
	if (!cuda_axisTestZ0(e1.y, e1.x, fey, fex, v0, v1, boxhalfsize, rad, min, max, p0, p1))
		return false;

	fex = fabsf(e2.x);
	fey = fabsf(e2.y);
	fez = fabsf(e2.z);

	if (!cuda_axisTestX2(e2.z, e2.y, fez, fey, v0, v1, boxhalfsize, rad, min, max, p0, p1))
		return false;
	if (!cuda_axisTestY1(e2.z, e2.x, fez, fex, v0, v1, boxhalfsize, rad, min, max, p0, p1))
		return false;
	if (!cuda_axisTestZ12(e2.y, e2.x, fey, fex, v1, v2, boxhalfsize, rad, min, max, p1, p2))
		return false;

	cuda_findMinMax(v0.x, v1.x, v2.x, min, max);
	if (min > boxhalfsize.x || max < -boxhalfsize.x)
		return false;

	cuda_findMinMax(v0.y, v1.y, v2.y, min, max);
	if (min > boxhalfsize.y || max < -boxhalfsize.y)
		return false;

	cuda_findMinMax(v0.z, v1.z, v2.z, min, max);
	if (min > boxhalfsize.z || max < -boxhalfsize.z)
		return false;

	normal = cross(e0, e1);
	if (!cuda_planeBoxOverlap(normal, v0, boxhalfsize))
		return false;

	return true;
}

//-----------------/ main kernel /-----------------//

__global__ void chunkInserter(const size_t* indexBuffer, const size_t numIndices, cudaTextureObject_t tex, uint16_t* blockBuffer, const int chunkX, const int chunkZ) {

	const size_t index = (size_t)threadIdx.x + (size_t)blockIdx.x * (size_t)blockDim.x;

	const Vec3 boxCenter = {
		chunkX * 16.0f + index % 16 + 0.5f,
		(index / 4096 + CUDA_minChunkY) * 16.0f + (index % 4096) / 256 + 0.5f,
		chunkZ * 16.0f + (index / 16) % 16 + 0.5f
	};

	const Vec3 boxHalfSize = { 0.5f, 0.5f, 0.5f };
	const Vec3 boxMin = sub(boxCenter, boxHalfSize);
	const Vec3 boxMax = add(boxCenter, boxHalfSize);

	for (size_t i = 0; i < numIndices; i++) {
		const Triangle* tri = &CUDA_triangles[indexBuffer[i]];

		if (cuda_approxTriBoxOverlap(boxMin, boxMax, CUDA_vertices[tri->vertexIndices[0]], CUDA_vertices[tri->vertexIndices[1]], CUDA_vertices[tri->vertexIndices[2]]) &&
			cuda_triBoxOverlap(boxCenter, boxHalfSize, CUDA_vertices[tri->vertexIndices[0]], CUDA_vertices[tri->vertexIndices[1]], CUDA_vertices[tri->vertexIndices[2]])){

			if (tri->blockID == UINT16_MAX) {
				const Vec3 s = sub(CUDA_vertices[tri->vertexIndices[1]], CUDA_vertices[tri->vertexIndices[0]]);

				const Vec3 t = sub(CUDA_vertices[tri->vertexIndices[2]], CUDA_vertices[tri->vertexIndices[0]]);

				const Vec3 n = cross(s, t);

				const Vec3 delta = sub(boxCenter, CUDA_vertices[tri->vertexIndices[0]]);

				const float nnInv = 1.0f / dot(n, n);
				const float w = dot(cross(s, delta), n) * nnInv;
				const float v = dot(cross(delta, t), n) * nnInv;
				const float u = 1.0f - w - v;

				const float x = u * CUDA_texCoords[tri->vertexIndices[0]].u + v * CUDA_texCoords[tri->vertexIndices[1]].u + w * CUDA_texCoords[tri->vertexIndices[2]].u;
				const float y = u * CUDA_texCoords[tri->vertexIndices[0]].v + v * CUDA_texCoords[tri->vertexIndices[1]].v + w * CUDA_texCoords[tri->vertexIndices[2]].v;

				const uchar4 color = tex2D<uchar4>(tex, x, y);

				size_t closestMatch = 0;
				uint64_t minDelta = UINT64_MAX;
				for (size_t j = 0; j < CUDA_lookupSize; j++) {

					const uint64_t delta = (uint64_t)abs(color.x - CUDA_lookupColors[j].r) +
						(uint64_t)abs(color.y - CUDA_lookupColors[j].g) +
						(uint64_t)abs(color.z - CUDA_lookupColors[j].b) +
						(uint64_t)abs(color.w - CUDA_lookupColors[j].a);

					if (delta < minDelta) {
						minDelta = delta;
						closestMatch = j;
					}
				}

				blockBuffer[index] = CUDA_lookupIndices[closestMatch];
			} else {
				blockBuffer[index] = tri->blockID;
			}
			break;
		}
	}
}

namespace CUDA {
	void insertBlocks(size_t numBlocks, size_t numThreads, size_t* indexBuffer, size_t numIndices, cudaTextureObject_t tex, uint16_t* blockBuffer, int chunkX, int chunkZ) {
		chunkInserter <<<numBlocks, numThreads>>> (indexBuffer, numIndices, tex, blockBuffer, chunkX, chunkZ);
	}
}
