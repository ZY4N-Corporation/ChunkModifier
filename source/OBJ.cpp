#include <OBJ.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

#include <mat3x3.hpp>
#include <Logger.hpp>


OBJ::OBJ(std::vector<vf3> _vertices, std::vector<texCoord> _texCoords, std::vector<indexTriangle> _triangles, std::vector<material> _materials, std::vector<Image> _textures) {
	texCoords = _texCoords;
	triangles = _triangles;
	materials = _materials;
	textures = _textures;
	vertices = _vertices;
	transformedVertices = std::vector<vf3>(vertices);
}


OBJ OBJ::loadOBJ(std::string path) {

	const size_t lastSlash = path.find_last_of("\\/");
	if (lastSlash == std::string::npos)
		throw std::runtime_error("[load_obj_error] cannot separate path from filename");

	std::string directory = path.substr(0, lastSlash + 1);
	std::string filename = path.substr(lastSlash + 1);
	
	std::vector<material> materials;	
	std::vector<Image> textures;
	std::vector<vf3> verticeList;
	std::vector<texCoord> textureCoordList;
	std::vector<indexTriangle> triangleList;

	//add default material
	materials.push_back(material{ "defaultMTL", { 0, 0, 0, 255 }, "minecraft:air", SIZE_MAX });

	bool foundMtlLib = false;
	int32_t materialIndex = 0;

	Logger::debug("loading OBJ...");

	try {
		std::ifstream input(directory + filename);
		if (!input.is_open())
			throw std::invalid_argument("Cannot open \"" + directory + filename + "\"");

		for (std::string line; getline(input, line);) {
			if (!foundMtlLib && line.rfind("mtllib ", 0) == 0) {
				std::string MTLfileName = line.substr(7);
				if(MTLfileName.rfind("./", 0) == 0)
					MTLfileName.replace(0, 2, directory);

				Logger::debug("found mtl file \"" + MTLfileName + "\"");
				
				try {
					loadMTL(MTLfileName, materials, textures);

					Logger::debug("Finished parsing mtl");

					foundMtlLib = true;
				} catch(std::exception &e) {
					Logger::error("Error while loading mtl " + std::string(e.what()));
				}
			}

			if (line.rfind("v ", 0) == 0) {
				vf3 vertex;
				if (sscanf(line.substr(2).c_str(), "%f %f %f", &vertex.x, &vertex.y, &vertex.z) != 3)
					throw std::invalid_argument("Format error whille parsing vertex");
				verticeList.push_back(vertex);
			}
			else if (line.rfind("vt ", 0) == 0) {
				vd2 coords;
				if (sscanf(line.substr(3).c_str(), "%lf %lf", &coords.x, &coords.y) != 2)
					throw std::invalid_argument("Format error whille parsing texture coordinate");
				textureCoordList.push_back({ coords, (uint8_t)materials[materialIndex].texIndex });
			}
			else if (line.rfind("usemtl ", 0) == 0) {
				std::string materialName = line.substr(7);
				materialIndex = -1;
				for (int32_t i = 0; i < materials.size(); i++) {
					if (materials[i].name == materialName) {
						materialIndex = i;
						break;
					}
				}
			} else if (line.rfind("f ", 0) == 0) {
				std::vector<size_t> vertexIndices;
				std::vector<size_t> textureCoordIndices;
				std::string cNum = "";
				uint8_t type = 0;

				for (size_t i = 2; i <= line.length(); i++) {
					if (i == line.length() || line[i] == ' ' || line[i] == '/') {

						int index = cNum.length() == 0 ? -1 : stoi(cNum) - 1;
						if (type == 0)
							vertexIndices.push_back(index);
						else if (type == 1)
							textureCoordIndices.push_back(index);

						if (i < line.length()) {
							cNum = "";
							if (line[i] == ' ') type = 0;
							else type++;
						}
					} else {
						cNum += line[i];
					}
				}

				if (materials[materialIndex].texIndex == -1) {
					for (size_t i = 2; i < vertexIndices.size(); i++) {
						triangleList.push_back(indexTriangle(
							vertexIndices[0], vertexIndices[i - 1], vertexIndices[i], -1, -1, -1, materialIndex
						));
					}
				} else {
					for (size_t i = 2; i < vertexIndices.size(); i++) {
						triangleList.push_back(indexTriangle(
							vertexIndices[0], vertexIndices[i - 1], vertexIndices[i],
							textureCoordIndices[0], textureCoordIndices[i - 1], textureCoordIndices[i],
							materialIndex
						));
					}
				}
			}
		}
	} catch (const std::runtime_error &e) {
		Logger::error("[corrutpt_file] " + std::string(e.what()));
	}

	Logger::debug("loaded OBJ");
	Logger::debug("found " + std::to_string(verticeList.size()) + " vertices");
	Logger::debug("found " + std::to_string(triangleList.size()) + " triangles");
	Logger::debug("found " + std::to_string(materials.size()) + " materials");

	return OBJ(verticeList, textureCoordList, triangleList, materials, textures);
}

void OBJ::loadMTL(std::string filename, std::vector<material>& materials, std::vector<Image>& textures) {

	std::string directory = filename.substr(0, filename.find_last_of("/\\") + 1ULL);

	std::ifstream lines(filename);

	if (lines.fail())
		throw std::runtime_error("Cannot find file \"" + filename + "\"");

	material m;
	for (std::string line; getline(lines, line);) {
		if (line.rfind("newmtl ", 0) == 0) {
			if (m.name.length() != 0) {
				materials.push_back(m);
			}
			m.name = line.substr(7);
		} else if (line.rfind("Kd ", 0) == 0) {
			float r, g, b = 0;
			if (sscanf(line.substr(3).c_str(), "%f %f %f", &r, &g, &b) != 3)
				throw std::runtime_error("Cannot parse color");

			m.c.r = static_cast<int>(255.0f * r);
			m.c.g = static_cast<int>(255.0f * g);
			m.c.b = static_cast<int>(255.0f * b);

		} else if (line.rfind("d ", 0) == 0) {
			m.c.a = static_cast<int>(255 * stof(line.substr(2)));
		} else if (line.rfind("blockID ", 0) == 0) {
			m.blockID = line.substr(8);
		} else if (line.rfind("map_Kd ", 0) == 0) {
			std::string imageName = line.substr(7);

			if (imageName.rfind("./", 0) == 0) {
				imageName.replace(0, 2, directory);
			} else if (imageName[1] != ':') {
				imageName = directory + imageName;
			}

			try {
				Logger::debug("Loading image " + imageName);
				textures.push_back(Image::load(imageName, 4));
				m.texIndex = textures.size() - 1ULL;
			} catch (const std::runtime_error& e) {
				Logger::error(e.what());
			}
		}
	}

	if (m.name.length() != 0) {
		materials.push_back(m);
	}
}

void OBJ::packTextures() {

	size_t numTextures = textures.size();

	if (numTextures < 2)
		return;

	Logger::debug("creating texture atlas\n");

	Position* texturePositions = nullptr;
	Image atlas = Image::createAtlas(textures.data(), numTextures, &texturePositions);

	textures.clear();
	textures.push_back(atlas);

	//-----------------/ replace texture materials with textureAtlas material /-----------------//
	
	for (material& m : materials) {
		if (m.texIndex != -1) {
			m.texIndex = 0;
		}
	}

	double invWidth = 1.0 / atlas.width;
	double invHeight = 1.0 / atlas.height;
	
	mat3x3* transformations = new mat3x3[numTextures];
	for (size_t i = 0; i < numTextures; i++) {
		transformations[i].scale({
			textures[i].width * invWidth,
			textures[i].height * invHeight
		});
		transformations[i].translate({
			texturePositions[i].x * invWidth,
			texturePositions[i].y * invHeight
		});
	}

	delete[] texturePositions;

	for (texCoord& tc : texCoords) {
		if (tc.texIndex != -1) {
			transformations[tc.texIndex].mult(tc.coords);
			tc.texIndex = 0;
		}
	}

	delete[] transformations;

}

const mat4x4& OBJ::getTransform() const {
	return transform;
}

const std::vector<Image>& OBJ::getTextures() const {
	return textures;
}

const std::vector<indexTriangle>& OBJ::getIndexTriangles() const {
	return triangles;
}

const std::vector<material>& OBJ::getMaterials() const {
	return materials;
}

mat4x4& OBJ::getTransform() {
	return transform;
}

std::vector<Image>& OBJ::getTextures() {
	return textures;
}

std::vector<indexTriangle>& OBJ::getIndexTriangles() {
	return triangles;
}

std::vector<material>& OBJ::getMaterials() {
	return materials;
}


std::vector<pointerTriangle> OBJ::createTriangleBuffer() const {

	transformVertices();

	std::vector<pointerTriangle> transformedTriangles(triangles.size());

	for (size_t i = 0; i < transformedTriangles.size(); i++) {
		const indexTriangle& t = triangles[i];

		for (size_t j = 0; j < 3; j++) {
			transformedTriangles[i].vertices[j] = &transformedVertices[t.vertexIndices[j]];
		}

		if (t.materialIndex != -1) {
			transformedTriangles[i].m = &materials[t.materialIndex];
			for (size_t j = 0; j < 3; j++) {
				transformedTriangles[i].texCoords[j] = &texCoords[t.texCoordIndices[j]].coords;
			}
		}
	}

	return transformedTriangles;
}


std::vector<Vec3> OBJ::createVertexBuffer() const {
	Vec3* ptr = (Vec3*)&transformedVertices[0];
	return std::vector<Vec3>(ptr, ptr + vertices.size());
}

std::vector<Vec2> OBJ::createTexCoordBuffer() const {

	std::vector<Vec2> texCoordBuffer(texCoords.size());

	for (size_t i = 0; i < texCoords.size(); i++) {
		texCoordBuffer[i].u = (float)texCoords[i].coords.x;
		texCoordBuffer[i].v = (float)texCoords[i].coords.y;
	}

	return texCoordBuffer;
}

void OBJ::calcMinMax(vf3 &min, vf3 &max) const {
	transformVertices();

	min = transformedVertices[0];
	max = transformedVertices[0];

	for (const vf3 &v : transformedVertices) {
		min.x = std::min(min.x, v.x);
		min.y = std::min(min.y, v.y);
		min.z = std::min(min.z, v.z);

		max.x = std::max(max.x, v.x);
		max.y = std::max(max.y, v.y);
		max.z = std::max(max.z, v.z);
	}
}

void OBJ::center() {
	vf3 min, max;
	calcMinMax(min, max);
	vf3 offset = (min + max) * -0.5f;
	transform.translate(offset);
	verticesTransformed = false;
}

void OBJ::transformVertices() const {
	if (!verticesTransformed) {
		for (size_t i = 0; i < vertices.size(); i++) {
			transformedVertices[i] = transform.multToNew(vertices[i]);
		}
		verticesTransformed = true;
	}
}

void OBJ::rotate(const vf3& rotation) {
	rotateX(rotation.x);
	rotateY(rotation.y);
	rotateZ(rotation.z);
}

void OBJ::rotateX(const float angle) {
	this->transform.apply(mat4x4::fromXAngle(static_cast<float>(angle * M_PI / 180)));
	verticesTransformed = false;
}

void OBJ::rotateY(const float angle) {
	this->transform.apply(mat4x4::fromYAngle(static_cast<float>(angle * M_PI / 180.0f)));
	verticesTransformed = false;
}

void OBJ::rotateZ(const float angle) {
	this->transform.apply(mat4x4::fromZAngle(static_cast<float>(angle * M_PI / 180.0f)));
	verticesTransformed = false;
}

void OBJ::scaleTo(const vf3 &destSize) {
	vf3 min, max;
	calcMinMax(min, max);
	scaleXYZ(destSize / (max - min));
	verticesTransformed = false;
}

void OBJ::scaleToX(const float x) {
	vf3 min, max;
	calcMinMax(min, max);
	scale(x / (max.x - min.x));
	verticesTransformed = false;
}

void OBJ::scaleToY(const float y) {
	vf3 min, max;
	calcMinMax(min, max);
	scale(y / (max.y - min.y));
	verticesTransformed = false;
}

void OBJ::scaleToZ(const float z) {
	vf3 min, max;
	calcMinMax(min, max);
	scale(z / (max.z - min.z));
	verticesTransformed = false;
}

void OBJ::scale(const float a) {
	transform.scale(a);
	verticesTransformed = false;
}

void OBJ::scaleXYZ(const vf3 &v) {
	transform.scale(v);
	verticesTransformed = false;
}

void OBJ::apply(const mat4x4 &m) {
	this->transform.apply(m);
	verticesTransformed = false;
}

void OBJ::translate(const vf3 &v) {
	transform.translate(v);
	verticesTransformed = false;
}
