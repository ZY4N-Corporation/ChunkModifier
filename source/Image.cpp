#include <Image.hpp>

#include <stbImageNoWarnings.hpp>
#include <algorithm>


Image::Image(const Image& img) {
	this->width = img.width;
	this->height = img.height;
	this->numChannels = img.numChannels;
	this->dataSize = img.dataSize;

	this->data = new uint8_t[dataSize];
	std::memcpy(this->data, img.data, dataSize);
}

Image::Image(Image&& img) noexcept {

	this->width = img.width;
	this->height = img.height;
	this->numChannels = img.numChannels;
	this->dataSize = img.dataSize;
	this->data = img.data;

	img.width = 0;
	img.height = 0;
	img.numChannels = 0;
	img.dataSize = 0;
	img.data = nullptr;
}


Image& Image::operator=(const Image& img) {
	delete[] data;

	this->width = img.width;
	this->height = img.height;
	this->numChannels = img.numChannels;
	this->dataSize = img.dataSize;

	this->data = new uint8_t[dataSize];
	std::memcpy(this->data, img.data, dataSize);

	return *this;
}

Image& Image::operator=(Image&& img) noexcept {
	if (this != &img) {
		delete[] data;

		this->width = img.width;
		this->height = img.height;
		this->numChannels = img.numChannels;
		this->dataSize = img.dataSize;
		this->data = img.data;

		img.width = 0;
		img.height = 0;
		img.numChannels = 0;
		img.dataSize = 0;
		img.data = nullptr;
	}
	return *this;
}

Image::~Image() {
	delete[] data;
}

Image Image::load(const std::string& filename, uint8_t forceChannels) {
	int width, height, channels;

	stbi_set_flip_vertically_on_load(true);
	uint8_t* data = static_cast<uint8_t*>(stbi_load(filename.c_str(), &width, &height, &channels, forceChannels));

	if (!data)
		throw std::runtime_error("[image] Cannot load image \"" + filename + "\"");

	if (forceChannels != 0)
		channels = forceChannels;

	return Image(static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint8_t>(channels), data);
}

Image Image::create(const uint32_t width, const uint32_t height, const uint8_t numChannels, const uint8_t* fill) {
	const uint64_t len = (uint64_t)width * (uint64_t)height * (uint64_t)numChannels;
	uint8_t* data = new uint8_t[len];
	
	for (uint32_t i = 0; i < len; i++)
		data[i] = fill[i % numChannels];
	
	return Image(width, height, numChannels, data);
}

Image Image::createAtlas(Image* images, size_t numTextures, Position** positions) {

	if (!(*positions))
		*positions = new Position[numTextures];


	stbrp_rect* rects = new stbrp_rect[numTextures];

	for (int i = 0; i < numTextures; i++) {
		rects[i] = {
			i,
			(stbrp_coord)images[i].width,
			(stbrp_coord)images[i].height,
			(stbrp_coord)0,
			(stbrp_coord)0,
			0
		};
	}
	bool success = false;

	uint64_t minPixels = 0;
	for (size_t i = 0; i < numTextures; i++) {
		minPixels += static_cast<uint64_t>(images[i].width) * static_cast<uint64_t>(images[i].height);
	}

	uint32_t width = (uint32_t)sqrt((double)minPixels);

	const constexpr uint32_t numTries = 4;
	for (uint32_t i = 0; (i < numTries) && !success; i++) {

		stbrp_context ctx;

		stbrp_node* nodes = new stbrp_node[width * 2U];
		std::memset(nodes, 0, 2U * width * sizeof(stbrp_node));

		stbrp_init_target(&ctx, (int32_t)width, (int32_t)width, nodes, (int32_t)width * 2);
		stbrp_pack_rects(&ctx, rects, (int32_t)numTextures);

		delete[] nodes;

		success = true;
		for (size_t i = 0; i < numTextures; ++i) {
			if (!rects[i].was_packed) {
				success = false;
				break;
			} else {
				(*positions)[i] = { rects[i].x, rects[i].y };
			}
		}

		width = (uint32_t)((float)width * 1.1);
	}

	if (success) {
		uint8_t fill[] = { 0, 255, 0, 255 };
		Image atlas = create(width, width, sizeof(fill), fill);

		for (size_t i = 0; i < numTextures; i++)
			atlas.set((uint32_t)rects[i].x, (uint32_t)rects[i].y, 4, images[i]);

		delete[] rects;

		return atlas;
	}

	delete[] rects;

	throw std::runtime_error("Does not pack");
}

int Image::save(const std::string& filename) const  {
	std::string type = filename.substr(filename.rfind('.') + 1, filename.length());
	std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return std::tolower(c); });

	int status = -1;

	if (type == "png") {
		status = stbi_write_png(filename.c_str(), width, height, numChannels, data, width * numChannels);
	} else if (type == "bmp") {
		status = stbi_write_bmp(filename.c_str(), width, height, numChannels, data);
	} else if (type == "tga") {
		status = stbi_write_tga(filename.c_str(), width, height, numChannels, data);
	} else if (type == "jpg" || type == "jpeg") {
		status = stbi_write_jpg(filename.c_str(), width, height, numChannels, data, width * numChannels);
	} else {
		throw std::invalid_argument("Fileformat \"" + type + "\" is unsupported");
	}

	return status;
}


void Image::set(uint32_t posX, uint32_t posY, uint8_t numChannels, const Image& img) {
	if (img.numChannels != numChannels)
		throw std::invalid_argument("Number of channels does not match");

	uint32_t stride = img.width * numChannels;
	for (size_t y = posY; y < posY + img.height; y++) {
		std::memcpy(&data[(posX + y * width) * numChannels], &img.data[(y - posY) * stride], stride);
	}
}

color Image::operator()(double x, double y) const {
	x = fmin(fmax(floor((width - 1.0) * x), 0.0), width - 1.0);
	y = fmin(fmax(floor((height - 1.0) * y), 0.0), height - 1.0);

	size_t index = static_cast<size_t>(x + width * y) * static_cast<size_t>(numChannels);

	return {
		data[index],
		data[index + 1],
		data[index + 2],
		static_cast<uint8_t>(numChannels > 3 ? data[index + 3] : 255)
	};
}

uint8_t* Image::pixels(size_t& pixelLen) {
	pixelLen = dataSize;
	return data;
}

color Image::averageColor() const {

	uint64_t* totalColors = new uint64_t[numChannels];
	std::memset(totalColors, 0, numChannels * sizeof(uint64_t));

	for (uint32_t i = 0; i < dataSize; i++)
		totalColors[i % numChannels] += data[i];

	uint64_t numPixels = width * height;

	color c{
		static_cast<uint8_t>(totalColors[0] / numPixels),
		static_cast<uint8_t>(totalColors[1] / numPixels),
		static_cast<uint8_t>(totalColors[2] / numPixels),
		static_cast<uint8_t>(numChannels > 3 ? totalColors[3] / numPixels : 255)
	};

	delete[] totalColors;

	return c;
}
