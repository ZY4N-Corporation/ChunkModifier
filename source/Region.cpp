#include <Region.hpp>

#include <vector>
#include <cstring>
#include <Logger.hpp>
#include <BEstream.hpp>
#include <Binary.hpp>

void Region::loadMCAtoBuffer(const std::string& filename, int x, int z, const vf3& min, const vf3& max,
	LockableQueue<Chunk> &inputBuffer, LockableQueue<Chunk> &outputBuffer) {

	size_t dataLen = 0;
	uint8_t* data = Binary::load(filename, dataLen);

	for (int chunkX = 0; chunkX < 32; chunkX++) {
		for (int chunkZ = 0; chunkZ < 32; chunkZ++) {
			try {
				size_t dataOffset = 0;
				uint8_t sectorCount = 0;

				if (dataLen != 0) {
					size_t headerOffset = ((size_t)chunkX + 32 * (size_t)chunkZ) * 4;

					const uint32_t sectorOffset = BEstream::read<uint32_t>(data, headerOffset, 3);

					dataOffset = (size_t)sectorOffset * 4096ULL;

					if (dataOffset > dataLen)
						throw std::runtime_error(std::string("[corrupt_file] chunk offset out of range: ") + std::to_string(dataOffset));

					sectorCount = data[headerOffset];
				}

				vf3 chunPos(x * 512.0f + chunkX * 16.0f, 0.0f, z * 512.0f + chunkZ * 16.0f);

				const bool toBeModified = (chunPos.x + 16.0f >= min.x && chunPos.x <= max.x &&
									chunPos.z + 16.0f >= min.z && chunPos.z <= max.z);

				if (sectorCount == 0) {
					if (toBeModified) {
						inputBuffer.push(Chunk::create(chunkType::VANILLA, (int)chunPos.x, (int)chunPos.z));
					}		
				} else {

					size_t chunkDataLen = BEstream::read<uint32_t>(data, dataOffset);

					uint8_t compressionType = data[dataOffset++];
					if (compressionType != 2)
						throw std::runtime_error("[corrupt_file] Unkown compression type " + std::to_string(compressionType));

					uint8_t* cunkData = new uint8_t[chunkDataLen];

					std::memcpy(cunkData, &data[dataOffset], chunkDataLen);
					
					Chunk chunk(chunkType::VANILLA, (int)chunPos.x, (int)chunPos.z, cunkData, chunkDataLen, true);
					
					if (toBeModified) inputBuffer.push(chunk);
					else outputBuffer.push(chunk);
				}
			} catch (const std::exception& e) {
				std::string error = std::string("Error while parsing chunk ");
				Logger::error(error + std::to_string(chunkX) + " " + std::to_string(chunkZ) + " " + std::string(e.what()));
			}
		}
	}

	delete[] data;
}

Region Region::loadMCA(const std::string& filename, int x, int z) {
	size_t dataLen = 0;
	uint8_t* data = Binary::load(filename, dataLen);

	Region out(x, z);

	if (dataLen == 0) {
		throw std::runtime_error("Region \"" + filename + "\" is empty or cannot be found");
	} else {
		for (int chunkX = 0; chunkX < 32; chunkX++) {
			for (int chunkZ = 0; chunkZ < 32; chunkZ++) {
				try {
					vf3 chunPos(x * 512.0f + chunkX * 16.0f, 0.0f, x * 512.0f + chunkZ * 16.0f);

					size_t headerOffset = ((size_t)chunkX + 32 * (size_t)chunkZ) * 4;

					uint32_t sectorOffset = BEstream::read<uint32_t>(data, headerOffset, 3);

					size_t dataOffset = (size_t)sectorOffset * 4096ULL;

					if (dataOffset > dataLen)
						throw std::runtime_error("[corrupt_file] chunk offset out of range");

					const uint8_t sectorCount = data[headerOffset];

					if (dataOffset != 0 && sectorCount != 0) {
						size_t chunkDataLen = BEstream::read<uint32_t>(data, dataOffset, 4);

						uint8_t compressionType = data[dataOffset++];
						if (compressionType != 2)
							throw std::runtime_error("[corrupt_file] Unkown compression type " + std::to_string(compressionType));

						uint8_t* cunkData = new uint8_t[chunkDataLen];

						std::memcpy(cunkData, &data[dataOffset + 1], chunkDataLen);

						out.chunks.push_back(Chunk(chunkType::VANILLA, (int)chunPos.x, (int)chunPos.z, cunkData, chunkDataLen, true));
					}

				} catch (const std::runtime_error& e) {
					std::string type("Error while parsing chunk ");
					Logger::error(type + std::to_string(chunkX) + " " + std::to_string(chunkZ) + " " + e.what());
				}
			}
		}
		delete[] data;
	}

	return out;
}

void Region::saveMCA(const std::string &filename) {

	uint32_t outputLen = 8192;
	for (const Chunk& chunk : chunks)
		if (chunk.dataSize > 0)
			outputLen += static_cast<uint32_t>(std::ceil((chunk.dataSize + 5) / 4096.f) * 4096);

	if (outputLen == 8192)
		return;

	uint8_t* output = new uint8_t[outputLen];

	std::memset(output, 0, 8192);

	size_t dataOffset = 2;

	for (const Chunk &chunk : chunks) {
		if (chunk.dataSize > 0) {

			//-------------/ header /-------------//

			size_t header_offset = 4ULL * (size_t)(chunk.x / 16 - x * 32 + (chunk.z / 16 - z * 32) * 32);

			BEstream::write(output, header_offset, &dataOffset, 3);

			uint8_t sectorCount = static_cast<uint8_t>(std::ceil((chunk.dataSize + 5) / 4096.0f));

			output[header_offset++] = sectorCount;

			//-------------/ timestamp /-------------//

			size_t timestampOffset = header_offset + 4092;
			uint32_t timestamp = static_cast<uint32_t>(std::time(0));

			BEstream::write(output, timestampOffset, &timestamp);


			//-------------/ data /-------------//

			size_t scaledDataOffset = dataOffset * 4096;

			uint32_t dataLen = (uint32_t)chunk.dataSize;
			BEstream::write(output, scaledDataOffset, &dataLen);

			output[scaledDataOffset++] = 2;
			
			std::memcpy(&output[scaledDataOffset], chunk.data, chunk.dataSize);

			dataOffset += sectorCount;
		}
	}

	Binary::save(output, outputLen, filename);

	delete[] output;
}
