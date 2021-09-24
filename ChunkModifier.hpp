#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Chunk.hpp>
#include <LockableQueue.hpp>
#include <OBJ.hpp>
#include <functional>
#include <vf3.hpp>
#include <mcBoundingBox.hpp>


class ChunkModifier {
protected:
	const mcBoundingBox workingVolume;
	static const std::string assetsPath;

public:
	ChunkModifier(const mcBoundingBox& _workingVolume) : workingVolume{ _workingVolume } {}

	virtual void modifyChunk(Chunk&) = 0;

	static void loadAVGColor(std::string filename, const std::function<void(color, const std::string&)>& insert);

	static void reassembleRegions(LockableQueue<Chunk>* inputBuffer, std::string outputDir, uint64_t numChunks, bool* active);
};
