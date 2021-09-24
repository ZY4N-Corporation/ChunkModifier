#pragma once

#include <bitset>
#include <stdexcept>
#include <string>

struct BEstream {
	uint8_t* buffer;
	const size_t bufferSize;
	size_t index;

	BEstream(uint8_t* input, const size_t inputLen) : buffer(input), bufferSize(inputLen), index(0) {}

	BEstream(const size_t inputLen) : buffer(new uint8_t[inputLen]), bufferSize(inputLen), index(0) {}

	template<typename T>
	BEstream& operator>>(T& dst) {
		if (index + sizeof(T) > bufferSize)
			throw std::out_of_range("size: " + std::to_string(bufferSize) + " index: " + std::to_string(index + sizeof(T)));

		uint8_t* bytes = reinterpret_cast<uint8_t*>(&dst);
		for (size_t i = sizeof(T); i > 0; bytes[--i] = buffer[index++]);
		return *this;
	}

	template<typename T>
	BEstream& operator<<(const T& src) {
		if (index + sizeof(T) > bufferSize)
			throw std::out_of_range("size: " + std::to_string(bufferSize) + " index: " + std::to_string(index + sizeof(T)));

		const uint8_t* tmp = reinterpret_cast<const uint8_t*>(&src);
		for (size_t i = sizeof(T); i > 0; buffer[index++] = tmp[--i]);
		return *this;
	}

	template<typename T>
	static T read(const uint8_t* buffer, size_t& index, const size_t forceSize = sizeof(T)) {
		uint8_t tmp[sizeof(T)];
		size_t i = 0;
		while (i < forceSize) tmp[forceSize - ++i] = buffer[index++];
		while (i < sizeof(T)) tmp[i++] = 0;
		return *(T*)tmp;
	}

	template<typename T>
	static void write(uint8_t* buffer, size_t& index, const T* src, size_t forceSize = sizeof(T)) {
		const uint8_t* bytes = reinterpret_cast<const uint8_t*>(src);
		while (forceSize > 0) buffer[index++] = bytes[--forceSize];
	}
};

template<>
inline BEstream& BEstream::operator>>(std::string& dst) {
	uint16_t length;
	*this >> length;

	if (index + length > bufferSize)
		throw std::out_of_range("[read_error] size: " + std::to_string(bufferSize) + " index: " + std::to_string(index + length));

	dst.resize(length);
	std::memcpy(&dst[0], &buffer[index], length);
	index += length;

	return *this;
}

template<>
inline BEstream& BEstream::operator<<(const std::string& src) {
	size_t strLen = src.length();
	if (index + strLen + sizeof(uint16_t) > bufferSize)
		throw std::out_of_range("size: " + std::to_string(bufferSize) + " index: " + std::to_string(index + strLen + sizeof(uint16_t)));
	
	*this << static_cast<uint16_t>(strLen);

	std::memcpy(&buffer[index], &src[0], strLen);
	index += strLen;

	return *this;
}

template<>
inline std::string BEstream::read(const uint8_t* buffer, size_t& index, const size_t forceSize) {
	uint16_t length = read<uint16_t>(buffer, index);
	const std::string ret(reinterpret_cast<const char*>(&buffer[index]));
	index += length;
	return ret;
}

template<>
inline void BEstream::write(uint8_t* buffer, size_t& index, const std::string* src, size_t forceSize) {
	size_t strLen = src->length();
	write(buffer, index, &strLen, sizeof(uint16_t));
	std::memcpy(&buffer[index], &src[0], strLen);
	index += strLen;
}
