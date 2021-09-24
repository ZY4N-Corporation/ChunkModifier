#include "ChunkModifier_GPU.hpp"

#include <iostream>
#include <fstream>
#include <bitset>

#include <ProgressBar.hpp>
#include <vd3.hpp>
#include <TriangleBoxIntersection.h>
#include <Logger.hpp>
#include <NBT.hpp>


ChunkModifier_GPU::ChunkModifier_GPU(const mcBoundingBox& workingVolume,
	std::vector<Vec3>&& _vertices,
	std::vector<Triangle>&& _triangles,
	cudaTextureObject_t* _texture,
	const std::string* _blockIDLookup,
	size_t _numBlockIDs)
	: ChunkModifier{ workingVolume }, vertices(std::move(_vertices)), triangles(std::move(_triangles)),
	texture{ _texture }, blockIDLookup{ _blockIDLookup }, numBlockIDs{ _numBlockIDs } {}


ChunkModifier_GPU::~ChunkModifier_GPU() {

	Logger::warn("Destructor called");
	delete[] blockIDLookup;

	checkCUDA(cudaDestroyTextureObject(*texture));

	CUDA::freeVertexBuffer();
	CUDA::freeTexCoordBuffer();
	CUDA::freeTriangleBuffer();
	CUDA::freeLookupColorBuffer();
	CUDA::freeLookupIndexBuffer();
}

ChunkModifier* ChunkModifier_GPU::init(OBJ& object, const mcBoundingBox& workingVolume) {

	std::string blockIDFilename = assetsPath + "blockIDlists/itemLookup.txt";

	std::ifstream fileIn(blockIDFilename);

	if (!fileIn)
		throw std::runtime_error("[block_parser_error] cannot find file \"" + blockIDFilename + "\"");

	std::vector<std::string> blockIDs;
	std::string line;
	while (std::getline(fileIn, line)) {
		blockIDs.push_back(line);
	}

	size_t numBlockIDs = blockIDs.size();
	std::string* blockIDLookup = new std::string[numBlockIDs];
	std::copy(blockIDs.begin(), blockIDs.end(), blockIDLookup);

	Logger::warn(std::to_string(numBlockIDs));

	//------------------------/ calc avg-Color /------------------------//

	Logger::log("calculating texture average color");

	ColorLookup<uint16_t> blocksByColor;
	ChunkModifier::loadAVGColor(assetsPath + "blockIDlists/blocks.txt", [&](color avgColor, const std::string& blockID) {
		blocksByColor.insert(avgColor, binSearch(blockIDLookup, numBlockIDs, blockID));
	});

	Logger::debug("using " + std::to_string(blocksByColor.size()) + " different blocks");

	std::vector<Vec3> vertices = object.createVertexBuffer();
	CUDA::initVertexBuffer(vertices);

	CUDA::initTexCoordBuffer(object.createTexCoordBuffer());

	CUDA::initLookupColorBuffer(blocksByColor.getKeys());
	CUDA::initLookupIndexBuffer(blocksByColor.getValues());
	CUDA::setLookupSize(blocksByColor.size());

	Logger::debug("creating index triangles");

	std::vector<material>& materials = object.getMaterials();
	uint16_t* materialBlockIDs = new uint16_t[materials.size()];

	for (size_t i = 0; i < materials.size(); i++) {
		if (materials[i].texIndex != -1) {
			materialBlockIDs[i] = -1;
		} else if (materials[i].blockID.length() == 0) {
			materialBlockIDs[i] = blocksByColor.get(materials[i].c);
		} else {
			materialBlockIDs[i] = binSearch(blockIDLookup, numBlockIDs, materials[i].blockID);
		}
	}

	const std::vector<indexTriangle>& indexTriangles = object.getIndexTriangles();
	std::vector<Triangle> triangles(indexTriangles.size());

	for (size_t i = 0; i < indexTriangles.size(); i++) {
		std::memcpy(triangles[i].vertexIndices, indexTriangles[i].vertexIndices, 3 * sizeof(size_t));
		std::memcpy(triangles[i].texCoordIndices, indexTriangles[i].texCoordIndices, 3 * sizeof(size_t));
		triangles[i].blockID = materialBlockIDs[indexTriangles[i].materialIndex];
	}

	delete[] materialBlockIDs;

	CUDA::initTriangleBuffer(triangles);

	Logger::debug("init texture atlas");

	object.packTextures();

	std::vector<Image>& textures = object.getTextures();
	cudaTextureObject_t* texture = CUDA::createTexture(textures[0]);

	CUDA::setMinChunkY(workingVolume.minSectionY);

	return new ChunkModifier_GPU(workingVolume, std::move(vertices), std::move(triangles), texture, blockIDLookup, numBlockIDs);
}


void ChunkModifier_GPU::modifyChunk(Chunk& chunk) {

	Logger::debug("|Bchunk |W" + std::to_string(chunk.x) + " " + std::to_string(chunk.z));

	//------------------------/ pre-filter triangles /------------------------//

	const Vec3 sectionMin{ static_cast<float>(chunk.x), 0.0f, static_cast<float>(chunk.z) };
	const Vec3 sectionMax{ sectionMin.x + 16.0f, 256.0f, sectionMin.z + 16.0f };

	std::vector<size_t> chunkIndexBuffer;

	for (size_t i = 0; i < triangles.size(); i++) {
		if (approxTriBoxOverlap2D(sectionMin, sectionMax, vertices[triangles[i].vertexIndices[0]], vertices[triangles[i].vertexIndices[1]], vertices[triangles[i].vertexIndices[2]])) {
			chunkIndexBuffer.push_back(i);
		}
	}

	if (chunkIndexBuffer.size() > 0) try {

		if (chunk.compressed)
			chunk.uncompress();

		NBT root = chunk.getNBT();
		chunk.clean();

		NBT& level = root["Level"];
		level["xPos"] = chunk.x / 16;
		level["zPos"] = chunk.z / 16;

		NBTlist& sections = level["Sections"];

		//------------------------/ extract blocks /------------------------//

		const auto findOrAddBlock = [](NBTlist& palette, const std::string& blockName) {
			for (size_t i = 0; i < palette.size(); i++) {
				if (static_cast<const std::string&>(palette[i]["Name"]) == blockName) {
					return (uint16_t)i;
				}
			}
			palette.push_back(NBTcompound{ { "Name", blockName } });
			return static_cast<uint16_t>(palette.size() - 1);
		};

		//------------------------/ init blockBuffer /------------------------//

		uint16_t* device_blockBuffer = 0;
		size_t* device_indexBuffer = 0;

		const size_t numSections = workingVolume.maxSectionY - workingVolume.minSectionY + 1ULL;
		const size_t blockBufferSize = 4096ULL * numSections * sizeof(uint16_t);
		uint16_t* blockBuffer = new uint16_t[blockBufferSize];

		for (size_t sectionY = workingVolume.minSectionY; sectionY <= workingVolume.maxSectionY; sectionY++) {

			auto setctionIt = sections.end();

			for (auto it = sections.begin(); it != sections.end(); it++) {
				if (it->at<int8_t>("Y") == sectionY) {
					setctionIt = it;
					break;
				}
			}

			if (setctionIt == sections.end()) {
				sections.push_back(NBTcompound{
					{ "Y", (int8_t)sectionY },
					{ "BlockLight", NBTbyteArray(2048, 1) }
					});

				setctionIt = --sections.end();
			}
			NBTcompound& section = *setctionIt;


			if (section.find("Palette") == section.end()) {
				section.insert({ "Palette", NBTlist{} });
			}
			NBTlist& palette = section["Palette"];

			if (section.find("BlockStates") == section.end()) {
				section.insert({ "BlockStates", NBTlongArray(256, 0) });
			}
			NBTlongArray& blocks = section["BlockStates"];

			if (blocks.size() == 0) {
				blocks.resize(256, 0);
			}

			const size_t bitsPerBlock = static_cast<size_t>(std::max(std::ceil(log2(palette.size())), 4.0));
			const size_t blocksPerLong = 64 / bitsPerBlock;

			findOrAddBlock(palette, "minecraft:air");

			//------------------------/ create palette lookup /------------------------//

			const size_t absoluteIndex = sectionY - workingVolume.minSectionY;

			const size_t paletteSize = palette.size();
			uint16_t* indexPalette = new uint16_t[paletteSize];

			for (size_t i = 0; i < paletteSize; i++)
				indexPalette[i] = (uint16_t)binSearch(blockIDLookup, numBlockIDs, palette[i]["Name"]);

			//------------------------/ getColors /------------------------//

			const int64_t mask = INT64_MAX >> (64 - bitsPerBlock);
			for (size_t i = 0; i < 4096; i++)
				blockBuffer[absoluteIndex * 4096ULL + i] = static_cast<uint16_t>(indexPalette[(blocks[i / blocksPerLong] >> (bitsPerBlock * (i % blocksPerLong))) & mask]);

			delete[] indexPalette;
		}

		//------------------------/ copy to gpu /------------------------//

		checkCUDA(cudaMalloc(&device_indexBuffer, chunkIndexBuffer.size() * sizeof(size_t)));
		checkCUDA(cudaMemcpy(device_indexBuffer, chunkIndexBuffer.data(), chunkIndexBuffer.size() * sizeof(size_t), cudaMemcpyHostToDevice));

		checkCUDA(cudaMalloc(&device_blockBuffer, blockBufferSize));
		checkCUDA(cudaMemcpy(device_blockBuffer, blockBuffer, blockBufferSize, cudaMemcpyHostToDevice));

		//------------------------/ convert /------------------------//

		size_t numThreads = 1024ULL;
		size_t numBlocks = 4ULL * numSections;


		//pls check if texture gets copied
		CUDA::insertBlocks(numBlocks, numThreads, device_indexBuffer, chunkIndexBuffer.size(), *texture, device_blockBuffer, chunk.x / 16, chunk.z / 16);

		checkCUDA(cudaDeviceSynchronize());

		//------------------------/ get data + cleanup /------------------------//

		checkCUDA(cudaFree(device_indexBuffer));

		checkCUDA(cudaMemcpy(blockBuffer, device_blockBuffer, blockBufferSize, cudaMemcpyDeviceToHost));

		checkCUDA(cudaFree(device_blockBuffer))

		//------------------------/ update blocks /------------------------//

		for (size_t sectionY = workingVolume.minSectionY; sectionY < workingVolume.maxSectionY; sectionY++) {

			auto setctionIt = sections.end();

			for (auto it = sections.begin(); it != sections.end(); it++) {
				if (static_cast<int8_t>((*it)["Y"]) == sectionY) {
					setctionIt = it;
					break;
				}
			}

			NBTcompound& section = *setctionIt;

			NBTlist& palette = section["Palette"];

			const size_t new_bitsPerBlock = static_cast<size_t>(std::max(std::ceil(log2(palette.size())), 4.0));
			const size_t new_blocksPerLong = 64 / new_bitsPerBlock;
			const size_t new_blockArraySize = static_cast<size_t>(std::ceil(4096.0f / new_blocksPerLong));

			NBTlongArray& blocks = section["BlockStates"];

			blocks.resize(new_blockArraySize);
			std::fill(blocks.begin(), blocks.end(), 0);

			const size_t absoluteIndex = (sectionY - workingVolume.minSectionY) * 4096ULL;

			for (size_t i = 0; i < 4096; i++) {
				int64_t paletteIndex = static_cast<int64_t>(findOrAddBlock(palette, blockIDLookup[blockBuffer[absoluteIndex + i]]));
				blocks[i / new_blocksPerLong] |= paletteIndex << ((i % new_blocksPerLong) * new_bitsPerBlock);
			}
		}

		delete[] blockBuffer;

		chunk.setNBT(root);

	} catch (const std::exception& e) {
		Logger::error(e.what());
	}
}

uint16_t ChunkModifier_GPU::binSearch(const std::string* values, size_t numValues, const std::string& key) {
	uint16_t left = 0;
	uint16_t right = numValues - 1;

	while (left <= right) {
		size_t middle = (static_cast<size_t>(left) + static_cast<size_t>(right)) / 2;
		int offset = key.compare(values[middle]);

		if (offset == 0) return static_cast<uint16_t>(middle);

		if (offset > 0) left = middle + 1;
		else right = middle - 1;
	}

	return UINT16_MAX;
}
