#pragma once

#include <string>
#include <stdexcept>

#include <Chunk.hpp>
#include <vf3.hpp>
#include <LockableQueue.hpp>


struct Region {
	int x, z;
	std::vector<Chunk> chunks;

	Region(int _x, int _z) : x(_x), z(_z) {};

	static Region loadMCA(const std::string &filename, int x, int z);

	static void loadMCAtoBuffer(const std::string& filename, int x, int z, const vf3& min, const vf3& max,
		LockableQueue<Chunk>& inputBuffer, LockableQueue<Chunk>& outputBuffer);

	void saveMCA(const std::string &path);
};
