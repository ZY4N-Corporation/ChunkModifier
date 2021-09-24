#include <ZLib.hpp>

#include <zlib.h>
#include <vector>

uint8_t* ZLib::compress(uint8_t* data, size_t &dataSize) {
	uint8_t* tmp = new uint8_t[dataSize];

	size_t uncompressedSize = dataSize;
	int status = ::compress(tmp, (uLong*)&uncompressedSize, data, (uLong)dataSize);
	if (status != Z_OK)
		throw std::runtime_error("[compression_error] " + errorToString(status));

	delete[] data;

	dataSize = uncompressedSize;
	uint8_t* output = new uint8_t[dataSize];
	memcpy(output, tmp, dataSize);
	delete[] tmp;

	return output;
}

uint8_t* ZLib::uncompress(uint8_t* input, size_t& inputSize) {
	static const constexpr size_t chunkSize = 16384;

	z_stream stream;
	size_t pOffset = 0;
	std::vector<uint8_t> uncompressed;

	memset(&stream, 0, sizeof(stream));

	int status = inflateInit(&stream);
	if (status != Z_OK)
		throw std::runtime_error("[decompression_error] " + errorToString(status));

	stream.next_in = (Bytef*)input;
	stream.avail_in = (uInt)inputSize;

	do {
		std::vector<uint8_t> buffer;

		buffer.resize(chunkSize, 0);
		stream.next_out = (Bytef*)buffer.data();
		stream.avail_out = chunkSize;

		status = inflate(&stream, 0);
		uncompressed.insert(uncompressed.end(), buffer.begin(), buffer.begin() + (stream.total_out - pOffset));
		pOffset = stream.total_out;

	} while (status == Z_OK);

	inflateEnd(&stream);

	if (status != Z_STREAM_END)
		throw std::runtime_error("[decompression_error] " + errorToString(status));

	delete[] input;
	inputSize = uncompressed.size();

	uint8_t* output = new uint8_t[inputSize];
	memcpy(output, uncompressed.data(), inputSize);

	return output;
}

std::string ZLib::errorToString(int status) {
	std::string type = "[zlib_error] ";
	switch (status) {
	case Z_ERRNO:
		return type + "output file not open";
	case Z_STREAM_ERROR:
		return type + "invalid compression level";
	case Z_DATA_ERROR:
		return type + "invalid or incomplete deflate data";
	case Z_MEM_ERROR:
		return type + "out of memory";
	case Z_BUF_ERROR:
		return type + "need more memory";
	case Z_VERSION_ERROR:
		return type + "zlib version mismatch!";
	default:
		return type + std::to_string(status);
	}
}
