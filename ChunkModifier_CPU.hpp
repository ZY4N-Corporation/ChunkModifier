#pragma once
#include "ChunkModifier.hpp"
#include <bitset>

#include <LockableQueue.hpp>
#include <OBJ.hpp>
#include <ColorLookup.hpp>


class ChunkModifier_CPU : public ChunkModifier {
private:
	const std::vector<pointerTriangle> triangles;
	const std::vector<Image>& textures;
	const ColorLookup<std::string> blockIDtoColor;

public:
	ChunkModifier_CPU(const mcBoundingBox& workingVolume, 
		std::vector<pointerTriangle>&& _triangles,
		const std::vector<Image>& _textures, 
		ColorLookup<std::string>&& lookup) : 
			ChunkModifier{ workingVolume },
			triangles{ std::move(_triangles) },
			textures{ _textures },
			blockIDtoColor{ std::move(lookup) }{}

	static ChunkModifier* init(OBJ&, const mcBoundingBox&);

	void modifyChunk(Chunk&) override;
};
