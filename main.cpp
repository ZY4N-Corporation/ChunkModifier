#include <vd3.hpp>
#include <OBJ.hpp>
#include <Logger.hpp>
#include <ArgParser.hpp>

#include "ChunkModifier_CPU.hpp"
#include "ChunkModifier_GPU.hpp"

void insertOBJ(const std::string&, const std::string&, OBJ&, uint32_t, bool);

int main(int argc, char* argv[]) {

	Logger::initColors();

	std::string inputDir, outputDir;
	int numThreads = 1;
	bool useCUDA = false;
	OBJ model;
	
	ArgParser args(argc, argv);

	try {

		args.parseStr("inputDir", true, [&inputDir](std::string& dir) {
			inputDir = dir;
			const char lastChar = dir[dir.length() - 1];
			inputDir += (lastChar == '\\' || lastChar == '/') ? "/" : "";
		});

		args.parseStr("outputDir", true, [&outputDir](std::string& dir) {
			outputDir = dir;
			const char lastChar = dir[dir.length() - 1];
			outputDir += (lastChar == '\\' || lastChar == '/') ? "/" : "";
		});
		
		args.parseStr("objDir", true, [&model](std::string& dir) {
			model = OBJ::loadOBJ(dir);
		});
		
		args.parse("numThreads", true, numThreads);
		args.parseInt("logLevel", false, Logger::setLogLevel);
		args.parseBool("center", false, std::bind(&OBJ::center, &model));
		args.parseVec("scaleTo", false, std::bind(&OBJ::scaleTo, &model, std::placeholders::_1));
		args.parseVec("scale", false, std::bind(&OBJ::scaleXYZ, &model, std::placeholders::_1));
		args.parseVec("rotate", false, std::bind(&OBJ::rotate, &model, std::placeholders::_1));
		args.parseVec("translate", false, std::bind(&OBJ::translate, &model, std::placeholders::_1));

		args.parse("CUDA", false, useCUDA);
		
	} catch (const std::exception& e) {
		Logger::error("[argument_parsing_error] " + std::string(e.what()));
		return -1; 
	}

	try {
		insertOBJ(inputDir, outputDir, model, numThreads, useCUDA);
	} catch (const std::exception& e) {
		Logger::error(e.what());
	}
}


void insertOBJ(const std::string& inputDir, const std::string& outputDir, OBJ& object, uint32_t numThreads, bool useCUDA) {

	Logger::log("calculating bounding box... ");

	vf3 minOBJ, maxOBJ;
	object.calcMinMax(minOBJ, maxOBJ);
	mcBoundingBox approxSize(minOBJ, maxOBJ);
	const uint64_t numChunks = approxSize.getNumChunks();


	Logger::log("inititalizing modifier... ");

	ChunkModifier* instance = (useCUDA ? ChunkModifier_GPU::init : ChunkModifier_CPU::init)(object, approxSize);


	Logger::log("launching workerthreads... ");

	bool waitForUnpacking = true;
	bool waitForModification = true;

	LockableQueue<Chunk> inputBuffer;
	LockableQueue<Chunk> outputBuffer;

	std::vector<std::thread> workerThreads(numThreads);
	for (auto& workerThread : workerThreads) {
		workerThread = std::thread([&]() {
			Chunk* chunk = nullptr;
			while (waitForUnpacking) {
				while (chunk = inputBuffer.pop()) {
					instance->modifyChunk(*chunk);
					outputBuffer.push(std::move(*chunk));
				}
			}
		});
	}
	
	std::thread reassmebler(ChunkModifier::reassembleRegions, &outputBuffer, outputDir, numChunks, &waitForModification);


	Logger::log("loading chunks... ");

	for (int regionX = approxSize.minRegionX; regionX <= approxSize.maxRegionX; regionX++) {
		for (int regionZ = approxSize.minRegionZ; regionZ <= approxSize.maxRegionZ; regionZ++) {
			std::string filename = inputDir + "r." + std::to_string(regionX) + "." + std::to_string(regionZ) + ".mca";
			Region::loadMCAtoBuffer(filename, regionX, regionZ, minOBJ, maxOBJ, inputBuffer, outputBuffer);
		}
	}


	Logger::debug("waiting for workerthreads... ");

	waitForUnpacking = false;

	for (auto& workerThread : workerThreads)
		workerThread.join();

	waitForModification = false;

	reassmebler.join();

	Logger::log("cleanup...");

	delete instance;
}
