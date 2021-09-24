#pragma once
#include <bitset>
#include <string>

struct color {
	uint8_t r, g, b, a;

	std::string toString() const {
		return "rgb(" + 
			std::to_string(r) + ", " +
			std::to_string(g) + ", " +
			std::to_string(b) + ", " +
			std::to_string(a) +
			")";
	}
};