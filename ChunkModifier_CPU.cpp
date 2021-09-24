#include "ChunkModifier_CPU.hpp"

#include <vd3.hpp>
#include <ProgressBar.hpp>
#include <Logger.hpp>
#include <TriangleBoxIntersection.h>

ChunkModifier* ChunkModifier_CPU::init(OBJ& model, const mcBoundingBox& workingVolume) {

	ColorLookup<std::string> blocksByColor;
	ChunkModifier::loadAVGColor(assetsPath + "blockIDlists/blocks.txt", 
		std::bind(&ColorLookup<std::string>::insert, &blocksByColor, std::placeholders::_1, std::placeholders::_2));

	std::vector<pointerTriangle> triangles = model.createTriangleBuffer();

	if (triangles.size() == 0)
		throw std::runtime_error("[obj_error] no triangles found");

	for (material& material : model.getMaterials()) {
		if (material.blockID.length() == 0) {
			material.blockID = blocksByColor.get(material.c);
		}
	}
	const std::vector<Image>& textures = model.getTextures();

	return new ChunkModifier_CPU(workingVolume, std::move(triangles), textures, std::move(blocksByColor));
}


void ChunkModifier_CPU::modifyChunk(Chunk& chunk) {
	Logger::debug("|Bchunk |W" + std::to_string(chunk.x) + " " + std::to_string(chunk.z));

	if (chunk.compressed)
		chunk.uncompress();

	NBT root = chunk.getNBT();
	chunk.clean();

	NBTcompound& level = root["Level"];
	level["xPos"] = chunk.x / 16;
	level["zPos"] = chunk.z / 16;

	NBTlist& sections = level["Sections"];
				
	for (size_t sectionY = workingVolume.minSectionY; sectionY < workingVolume.maxSectionY; sectionY++) try {

		const vf3 sectionMin(static_cast<float>(chunk.x), sectionY * 16.0f, static_cast<float>(chunk.z));
		const vf3 sectionMax = sectionMin + vf3(16.0f, 16.0f, 16.0f);

		std::vector<const pointerTriangle*> sectionTriangles;

		for (size_t i = 0; i < triangles.size(); i++)
			if (approxTriBoxOverlap(triangles[i].vertices, sectionMin, sectionMax))
				sectionTriangles.push_back(&triangles[i]);

		if (sectionTriangles.size() == 0)
			continue;
	
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

		//------------------------/ extract blockStates /------------------------//

		const uint32_t bitsPerBlock = static_cast<uint32_t>(std::max(std::ceil(log2(palette.size())), 4.0));
		const uint8_t blocksPerLong = 64 / bitsPerBlock;

		const auto findOrAddBlock = [&palette](const std::string& blockName) {
			for (size_t i = 0; i < palette.size(); i++) {
				if (static_cast<const std::string&>(palette[i]["Name"]) == blockName) {
					return (uint16_t)i;
				}
			}
			palette.push_back(NBTcompound{ { "Name", blockName } });
			return static_cast<uint16_t>(palette.size() - 1);
		};

		findOrAddBlock("minecraft:air");

		uint16_t blockIndices[4096];

		const int64_t mask = INT64_MAX >> (64 - bitsPerBlock);
		for (uint32_t i = 0; i < 4096; i++)
			blockIndices[i] = static_cast<uint16_t>((blocks[i / blocksPerLong] >> (bitsPerBlock * (i % blocksPerLong))) & mask);

		//------------------------/ insert object /------------------------//

		const float dim = 1.0f;
		const vf3 boxSize(dim, dim, dim);
		const vf3 boxHalfsize = boxSize * 0.5f;
		const vf3 boxMiddle(0.5f, 0.5f, 0.5f);

		for (uint8_t x = 0; x < 16; x++) {
			for (uint8_t y = 0; y < 16; y++) {
				for (uint8_t z = 0; z < 16; z++) {

					const vf3 box_pos(static_cast<float>(chunk.x + x), sectionY * 16.0f + y, static_cast<float>(chunk.z + z));
					const vf3 box_center = box_pos + boxMiddle;

					for (const pointerTriangle* triangle : sectionTriangles) {
						if (approxTriBoxOverlap(triangle->vertices, box_pos, box_pos + boxSize) &&
							triBoxOverlap(box_center, boxHalfsize, *(triangle->vertices[0]), *(triangle->vertices[1]), *(triangle->vertices[2]))
							) {

							if (triangle->m && triangle->m->texIndex != -1) {

								const vd3 s(*(triangle->vertices[1]) - *(triangle->vertices[0]));
								const vd3 t(*(triangle->vertices[2]) - *(triangle->vertices[0]));
								const vd3 n = s.cross(t);

								const vd3 delta(box_center - *(triangle->vertices[0]));

								const double invDet = 1.0 / n.dot(n);
								const double w = s.cross(delta).dot(n) * invDet;
								const double v = delta.cross(t).dot(n) * invDet;
								const double u = 1.0 - w - v;

								blockIndices[x + z * 16 + y * 256] = findOrAddBlock(blockIDtoColor.get(textures[triangle->m->texIndex](
									u* triangle->texCoords[0]->x + v * triangle->texCoords[1]->x + w * triangle->texCoords[2]->x,
									u* triangle->texCoords[0]->y + v * triangle->texCoords[1]->y + w * triangle->texCoords[2]->y
								)));

							} else {
								blockIndices[x + z * 16 + y * 256] = findOrAddBlock(triangle->m->blockID);
							}

							break;
						}
					}
				}
			}
		}

		//------------------------/ update blockStates /------------------------//

		const uint32_t new_bitsPerBlock = static_cast<uint32_t>(std::max(std::ceil(log2(palette.size())), 4.0));
		const uint32_t new_blocksPerLong = 64 / new_bitsPerBlock;
		const uint32_t new_blockArraySize = static_cast<uint32_t>(std::ceil(4096.0f / new_blocksPerLong));

		blocks.resize(new_blockArraySize);
		std::fill(blocks.begin(), blocks.end(), 0);

		for (size_t i = 0; i < 4096; i++) {
			blocks[i / new_blocksPerLong] |= static_cast<int64_t>(blockIndices[i]) << ((i % new_blocksPerLong) * new_bitsPerBlock);
		}

	} catch (const std::exception& e) {
		Logger::error(std::string("Error while modifiyng section: ") + e.what());

	}

	chunk.setNBT(root);
}
