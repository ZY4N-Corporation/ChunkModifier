#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include <mat4x4.hpp>
#include <vf3.hpp>
#include <vd2.hpp>
#include <Image.hpp>
#include <cStructs.h>

struct material {
	std::string name;
	color c { 0, 0, 0, 255 };
	std::string blockID;
	size_t texIndex { SIZE_MAX };
};

struct texCoord {
	vd2 coords;
	uint8_t texIndex;
};

struct indexTriangle {
	size_t vertexIndices[3];
	size_t texCoordIndices[3];
	size_t materialIndex;

	indexTriangle(size_t v1, size_t v2, size_t v3, size_t t1, size_t t2, size_t t3, size_t m) {
		vertexIndices[0] = v1;
		vertexIndices[1] = v2;
		vertexIndices[2] = v3;
		texCoordIndices[0] = t1;
		texCoordIndices[1] = t2;
		texCoordIndices[2] = t3;
		materialIndex = m;
	}
};

struct pointerTriangle {
	vf3 const *vertices[3];
	vd2 const *texCoords[3];
	material const *m;
};

class OBJ {
private:
	std::vector<vf3> vertices;
	std::vector<texCoord> texCoords;
	std::vector<material> materials;
	std::vector<Image> textures;
	std::vector<indexTriangle> triangles;
	
	mat4x4 transform;

	mutable std::vector<vf3> transformedVertices;
	mutable bool verticesTransformed;

public:

	OBJ(std::vector<vf3> vertices, 
		std::vector<texCoord> texCoords, 
		std::vector<indexTriangle> triangles,
		std::vector<material> materials, 
		std::vector<Image> textures);
	
	OBJ() = default;

	static OBJ loadOBJ(std::string filename);

	static void loadMTL(std::string filename, std::vector<material> &materials, std::vector<Image> &textures);

	void packTextures();

	void calcMinMax(vf3& min, vf3& max) const;

	mat4x4& getTransform();

	std::vector<material>& getMaterials();

	std::vector<indexTriangle>& getIndexTriangles();

	std::vector<Image>& getTextures();


	const mat4x4& getTransform() const;

	const std::vector<material>& getMaterials() const;

	const std::vector<indexTriangle>& getIndexTriangles() const;

	const std::vector<Image>& getTextures() const;


	std::vector<pointerTriangle> OBJ::createTriangleBuffer() const;

	std::vector<Vec3> createVertexBuffer() const;

	std::vector<Vec2> createTexCoordBuffer() const;

	void transformVertices() const;

	void center();
	
	void apply(const mat4x4& m);

	void rotate(const vf3& rotation);
	void rotateX(const float angle);
	void rotateY(const float angle);
	void rotateZ(const float angle);

	void scale(const float a);
	void scaleXYZ(const vf3 &v);
	void scaleTo(const vf3 &size);

	void scaleToX(const float x);
	void scaleToY(const float y);
	void scaleToZ(const float z);

	void translate(const vf3 &v);
};
