#pragma once

#include <vf3.hpp>
#include <cmath>

struct mcBoundingBox {
	const int minRegionX, maxRegionX;
	const int minRegionZ, maxRegionZ;

	const int minChunkX, maxChunkX;
	const int minChunkZ, maxChunkZ;

	const uint8_t minSectionY, maxSectionY;

	mcBoundingBox(const vf3& min, const vf3& max) :
		minRegionX{ static_cast<int>(std::floor(min.x / 512.0f)) },
		maxRegionX{ static_cast<int>(std::ceil(max.x / 512.0f)) },
		minRegionZ{ static_cast<int>(std::floor(min.z / 512.0f)) },
		maxRegionZ{ static_cast<int>(std::ceil(max.z / 512.0f)) },
		minChunkX{ static_cast<int>(std::floor(min.x / 16.0f)) },
		maxChunkX{ static_cast<int>(std::ceil(max.x / 16.0f)) },
		minChunkZ{ static_cast<int>(std::floor(min.z / 16.0f)) },
		maxChunkZ{ static_cast<int>(std::ceil(max.z / 16.0f)) },
		minSectionY{ static_cast<uint8_t>(std::min(std::max(std::floor(min.y / 16.0f), 0.0f), 16.0f)) },
		maxSectionY{ static_cast<uint8_t>(std::min(std::max(std::ceil(max.y / 16.0f), 0.0f), 16.0f)) } {}


	uint64_t getNumRegions() const {
		return static_cast<uint64_t>(maxRegionX - minRegionX) * static_cast<uint64_t>(maxRegionZ - minRegionZ);
	}

	uint64_t getNumChunks() const {
		return static_cast<uint64_t>(maxChunkX - minChunkX) * static_cast<uint64_t>(maxChunkZ - minChunkZ);
	}

	uint64_t getNumSections() const {
		return static_cast<uint64_t>(maxChunkX - minChunkX) * static_cast<uint64_t>(maxChunkZ - minChunkZ) * static_cast<uint64_t>(maxSectionY - minSectionY);
	}
};
