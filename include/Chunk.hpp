#pragma once
#include <bitset>
#include <stdexcept>
#include <string>

#include <ZLib.hpp>
#include <NBT.hpp>

enum class chunkType : uint8_t {
    VANILLA = 0,
	CUBIC = 1,
	SCHMEATIC = 2
};

struct Chunk {
	int x;
	int z;

	uint8_t* data;
	size_t dataSize;
	bool compressed;

	chunkType type;

	Chunk(chunkType _type) : type(_type), x(0), z(0), data(nullptr), dataSize(0), compressed(false) {};

	Chunk(chunkType _type, int _x, int _z, uint8_t* _data, size_t _dataSize, bool _compressed) : type(_type), x(_x), z(_z), data(_data), dataSize(_dataSize), compressed(_compressed) {};

	Chunk(const Chunk&);

	Chunk(Chunk&&) noexcept;


	static Chunk create(chunkType type, int x, int z);


	Chunk& operator=(const Chunk&);

	Chunk& operator=(Chunk&&) noexcept;

	~Chunk();

	void compress();
	void uncompress();

	NBT getNBT();
	void setNBT(const NBT& chunkData);

	std::string toString();

	void clean();
};

