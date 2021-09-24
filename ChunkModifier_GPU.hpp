#pragma once
#include "ChunkModifier.hpp"

#include <Region.hpp>
#include <ColorLookup.hpp>
#include <LockableQueue.hpp>
#include <OBJ.hpp>
#include "kernel.cuh"

class ChunkModifier_GPU : public ChunkModifier{
private:
	std::vector<Vec3> vertices;
	std::vector<Triangle> triangles;
	cudaTextureObject_t* texture;

	const std::string* blockIDLookup;
	size_t numBlockIDs;


public:
	ChunkModifier_GPU(const mcBoundingBox& workingVolume,
		std::vector<Vec3>&& _vertices,
		std::vector<Triangle>&& _triangles,
		cudaTextureObject_t* _texture,
		const std::string* _blockIDLookup,
		size_t _numBlockIDs);

	~ChunkModifier_GPU();

	static ChunkModifier* init(OBJ& object, const mcBoundingBox& workingVolume);

	void modifyChunk(Chunk&) override;
	
	static uint16_t binSearch(const std::string* values, const size_t numValues, const std::string& key);
};
