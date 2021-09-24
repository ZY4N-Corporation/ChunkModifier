#pragma once
#include <bitset>
#include <string>
#include <stdexcept>

namespace ZLib {
	
	uint8_t* compress(uint8_t* input, size_t& inputSize);

	uint8_t* uncompress(uint8_t* input, size_t& inputSize);

	std::string errorToString(int status);

};
