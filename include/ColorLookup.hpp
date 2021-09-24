#pragma once

#include <string>
#include <vector>
#include <color.h>

template<typename T>
class ColorLookup {
private:

	std::vector<color> keys;
	std::vector<T> values;
	size_t len = 0;

public:
	ColorLookup() : len{ 0 } {};

	size_t size() const {
		return len;
	}

	void insert(const color key, const T& value) {
		keys.push_back(key);
		values.push_back(value);
		len++;
	}

	void remove(size_t index) {
		keys.erase(keys.begin() + index);
		values.erase(values.begin() + index);
		len--;
	}

	T get(const color& key) const {
		uint64_t minDelta = UINT64_MAX;
		size_t index = 0;
		for (size_t i = 0; i < len; i++) {
			uint64_t delta = static_cast<uint64_t>(std::abs(key.r - keys[i].r)) +
				static_cast<uint64_t>(std::abs(key.g - keys[i].g)) +
				static_cast<uint64_t>(std::abs(key.b - keys[i].b)) +
				static_cast<uint64_t>(std::abs(key.a - keys[i].a));

			if (delta < minDelta) {
				minDelta = delta;
				index = i;
				if (delta == 0)
					break;
			}
		}
		return values[index];
	}

	std::vector<color>& getKeys() {
		return keys;
	}

	std::vector<T>& getValues() {
		return values;
	}

	std::string toString() const {
		std::string out;
		for (size_t i = 0; i < len; i++) {
			out += keys[i].toString() + " " + values[i] + "\n";
		}
		return out;
	}
};

template<>
inline std::string ColorLookup<std::string>::toString() const {
	std::string out;
	for (size_t i = 0; i < len; i++) {
		out += keys[i].toString() + " " + values[i] + "\n";
	}
	return out;
}

template<>
inline std::string ColorLookup<std::uint16_t>::toString() const {
	std::string out;
	for (size_t i = 0; i < len; i++) {
		out += keys[i].toString() + " " + std::to_string(values[i]) + "\n";
	}
	return out;
}
