#pragma once
#include <bitset>
#include <string>
#include <stdexcept>

#include <color.h>

struct Position {
	size_t x, y;
};

class Image {
private:
	size_t dataSize;
	uint8_t* data;

public:
	uint32_t width,height;
	uint8_t numChannels;


	Image() : width{ 0 }, height{ 0 }, numChannels{ 0 }, dataSize{ 0 }, data{ nullptr } {}

	Image(uint32_t _width, uint32_t _height, uint8_t _numChannels, uint8_t* _data) :
		width(_width), height(_height), numChannels(_numChannels), 
		dataSize((uint64_t)_width * (uint64_t)_height* (uint64_t)_numChannels), data(_data) {};

	Image(const Image&);

	Image(Image&&) noexcept;

	~Image();

	Image& operator=(const Image&);

	Image& operator=(Image&&) noexcept;

	static Image load(const std::string& filename, uint8_t forceChannels = 0);

	static Image create(const uint32_t width, const uint32_t height, const uint8_t numChannels, const uint8_t* fill);

	static Image createAtlas(Image* images, size_t numTextures, Position** positions);

	int save(const std::string& filename) const;

	color operator()(double x, double y) const;

	uint8_t* pixels(size_t& pixelLen);

	void set(uint32_t posX, uint32_t posY, uint8_t numChannels, const Image& img);

	color averageColor() const;
};
