#include "Chunk.hpp"

Chunk Chunk::create(chunkType type, int x, int z) {

	if (type == chunkType::VANILLA) {
		
		NBTlongArray heightmap(37);
		std::fill(heightmap.begin(), heightmap.end(), 1137128059338260031LL);

		NBT nbt = NBTcompound{
			{ "DataVersion", 2586 },
			{ "Level",
				NBTcompound{
					{ "xPos", x / 16 },
					{ "zPos", z / 16 },
					{ "Sections", NBTlist{} },
					{ "Heightmaps",
						NBTcompound{
							{ "OCEAN_FLOOR", NBTlongArray{} },
							{ "MOTION_BLOCKING_NO_LEAVES", NBTlongArray{} },
							{ "MOTION_BLOCKING", NBTlongArray{} },
							{ "WORLD_SURFACE", NBTlongArray(37,  1137128059338260031LL) },
						}
					}
				},
			},
			{ "CarvingMasks", NBTcompound{} },
			{ "Entities", NBTlist{} },
			{ "TileEntities", NBTlist{} },
			{ "TileTicks", NBTlist{} },
			{ "ToBeTicked", NBTlist{} },
			{ "Structures", NBTcompound{} },
			{ "InhabitedTime", 0LL },
			{ "LastUpdate", 0LL },
			{ "Status", NBTstring("full") }
		};
		
		size_t dataSize;
		uint8_t* data = nbt.serialize(dataSize);

		return Chunk(chunkType::VANILLA, x, z, data, dataSize, false);
	} else {
		throw std::invalid_argument("This type of chunk has not been implemented yet");
	}
}

Chunk::Chunk(const Chunk& chunk) {
	this->type = chunk.type;
	this->x = chunk.x;
	this->z = chunk.z;
	this->compressed = chunk.compressed;
	this->dataSize = chunk.dataSize;
	
	if (chunk.data) {
		this->data = new uint8_t[dataSize];
		std::memcpy(this->data, chunk.data, dataSize);
	} else data = nullptr;
}

Chunk::Chunk(Chunk&& chunk) noexcept {
	this->type = chunk.type;
	this->x = chunk.x;
	this->z = chunk.z;
	this->compressed = chunk.compressed;
	this->dataSize = chunk.dataSize;
	this->data = chunk.data;
	
	chunk.data = nullptr;
	chunk.dataSize = 0;
}


Chunk& Chunk::operator=(const Chunk& chunk) {
	type = chunk.type;
	x = chunk.x;
	z = chunk.z;
	compressed = chunk.compressed;
	dataSize = chunk.dataSize;

	if (chunk.data) {
		data = new uint8_t[dataSize];
		std::memcpy(data, chunk.data, dataSize);
	} else data = nullptr;
	
	return *this;
}


Chunk& Chunk::operator=(Chunk&& chunk) noexcept {
	if (this != &chunk) {
		delete[] this->data;

		this->type = chunk.type;
		this->x = chunk.x;
		this->z = chunk.z;
		this->compressed = chunk.compressed;
		this->dataSize = chunk.dataSize;
		this->data = chunk.data;

		chunk.data = nullptr;
		chunk.dataSize = 0;
	}

	return *this;
}


Chunk::~Chunk() {
	clean();
}

void Chunk::compress() {
	data = ZLib::compress(data, dataSize);
	compressed = true;
}

void Chunk::uncompress() {
	data = ZLib::uncompress(data, dataSize);
	compressed = false;
}

NBT Chunk::getNBT() {
	return std::move(NBT::parse(data, dataSize));
}

void Chunk::setNBT(const NBT& nbt) {
	clean();
	data = nbt.serialize(dataSize);
}

std::string Chunk::toString() {
	if (compressed) uncompress();
	NBT root = NBT::parse(data, dataSize);
	std::stringstream out;
	root.toString(out, 0);
	return out.str();
}

void Chunk::clean() {
	delete[] data;
	data = nullptr;
	dataSize = 0;
}
