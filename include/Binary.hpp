#pragma once

#include <bitset>
#include <string>
#include <fstream>
#include <Logger.hpp>

namespace Binary {

	inline uint8_t* load(const std::string& filename, size_t& len) {
		Logger::debug("loading \"" + filename + "\"...");

#ifdef _MSC_VER
		FILE* file;
		if (fopen_s(&file, filename.c_str(), "rb") != 0) {
			len = 0;
			Logger::debug("|Rfailed loading \"" + filename + "\"");
			return NULL;
		}
#else
		FILE* file = fopen(filename.c_str(), "rb");
		if (file == NULL) {
			len = 0;
			Logger::debug("|Rfailed loading \"" + filename + "\"");
			return NULL;
		}
#endif

		fseek(file, 0, SEEK_END);
		len = ftell(file);
		fseek(file, 0L, SEEK_SET);

		uint8_t* byteArray = new uint8_t[len];
		for (size_t i = 0; i < len; i++) {
			byteArray[i] = fgetc(file);
		}
		fclose(file);

		return byteArray;
	}

	inline void save(const uint8_t* data, size_t len, const std::string& filename) {
		Logger::debug("saving \"" + filename + "\"");
		auto myfile = std::ofstream(filename, std::ios::out | std::ios::binary);
		myfile.write(reinterpret_cast<const char*>(data), len);
		myfile.close();
	}
}
