#include "ChunkModifier.hpp"

#include <stdexcept>
#include <functional>
#include <fstream>

#include <Region.hpp>
#include <ProgressBar.hpp>
#include <Logger.hpp>

const std::string ChunkModifier::assetsPath = std::filesystem::current_path().string() + "/assets/";


void ChunkModifier::loadAVGColor(std::string filename, const std::function<void(color, const std::string&)>& insert) {

	const size_t lastSlash = filename.find_last_of("\\/");
	if (lastSlash == std::string::npos)
		throw std::runtime_error("[loadAVGColor_error] cannot separate path from filename");

	std::string directory = filename.substr(0, lastSlash + 1);
	filename = filename.substr(lastSlash + 1);

	std::ifstream fileIn(directory + filename);
	std::string outBuffer;
	bool changedFile = false;

	if (!fileIn) {
		throw std::runtime_error("[block_parser_error] cannot find file \"" + filename + "\"");
	}

	std::string line;
	while (std::getline(fileIn, line)) {

		std::vector<size_t> commas;
		for (size_t i = 0; (i = line.find(", ", i)) != std::string::npos; i++) {
			commas.push_back(i);
		}

		std::string textureName;
		switch (commas.size()) {
		case 1: textureName = line.substr(commas[0] + 2ULL, line.length() - commas[0]); break;
		case 2: textureName = line.substr(commas[0] + 2ULL, (size_t)(commas[1] - commas[0] - 2ULL)); break;
		default: throw std::invalid_argument("[block_parser_error] could not parse blockkID and/or textureName \"" + line + "\"");
		}

		const std::string blockID = line.substr(0, commas[0]);
		color avgColor;

		if (commas.size() == 1 || sscanf(line.substr(commas[1] + 2ULL).c_str(), "%hhu %hhu %hhu %hhu", &avgColor.r, &avgColor.g, &avgColor.b, &avgColor.a) != 4) {

			avgColor = Image::load(directory + textureName).averageColor();

			outBuffer += blockID + ", " + textureName + ", " +
				std::to_string(avgColor.r) + " " +
				std::to_string(avgColor.g) + " " +
				std::to_string(avgColor.b) + " " +
				std::to_string(avgColor.a) + "\n";

			changedFile = true;

		} else {
			outBuffer += line + "\n";
		}

		insert(avgColor, blockID);
	}

	fileIn.close();

	if (changedFile) {
		std::ofstream fileOut(filename);
		fileOut << outBuffer;
		fileOut.flush();
		fileOut.close();
	}
}

void ChunkModifier::reassembleRegions(LockableQueue<Chunk>* inputBuffer, std::string outputDir, uint64_t numChunks, bool* active) {

	Logger::debug("|K:::|Gstarting |Yassembler|K:::");

	std::vector<Region> regions;

	ProgressBar progress("modifying regions", 60, "\u001b[33;1m");
	uint64_t numReceivedChunks = 0;

	Chunk* chunk = nullptr;

	while (*active) {
		while (chunk = inputBuffer->pop()) {

			progress.setProgress((float)(++numReceivedChunks) / numChunks);
			progress.update();

			const int regionX = static_cast<int>(std::floor(chunk->x / 512.0f));
			const int regionZ = static_cast<int>(std::floor(chunk->z / 512.0f));

			auto regionIt = regions.end();

			for (auto it = regions.begin(); it != regions.end(); it++) {
				if (it->x == regionX && it->z == regionZ) {
					regionIt = it;
					break;
				}
			}

			if (regionIt == regions.end()) {
				regions.push_back(Region(regionX, regionZ));
				regionIt = --regions.end();
			}

			if (!chunk->compressed)
				chunk->compress();

			regionIt->chunks.push_back(std::move(*chunk));

			if (regionIt->chunks.size() == 1024) {
				regions[0].saveMCA(outputDir + "r." + std::to_string(regions[0].x) + "." + std::to_string(regions[0].z) + ".mca");
				regions.erase(regionIt);
			}
		}
	}

	//------------------------/ save remaining regions /------------------------//

	Logger::debug("\nflushing regionBuffer...");

	while (regions.size() > 0) {
		regions[0].saveMCA(outputDir + "r." + std::to_string(regions[0].x) + "." + std::to_string(regions[0].z) + ".mca");
		regions.erase(regions.begin());
	}

	Logger::debug("|K:::|rclsoing |Yassembler|K:::");
}
