#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

// OpenGL and FreeGlut headers.
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM.
#include <GLM/glm.hpp>
#include <GLM/gtc/type_ptr.hpp>

// C++ STL headers.
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>

// header file to use
#include <unordered_map>

// VertexPTN Declarations.
struct VertexPTN
{
	VertexPTN()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
		texcoord = glm::vec2(0.0f, 0.0f);
	}
	VertexPTN(glm::vec3 p)
	{
		position = p;
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
		texcoord = glm::vec2(0.0f, 0.0f);
	}
	VertexPTN(glm::vec3 p, glm::vec3 n, glm::vec2 uv)
	{
		position = p;
		normal = n;
		texcoord = uv;
	}

	void print() const
	{
		printf("P/T/N: [%f, %f, %f] , [%f, %f, %f] , [%f, %f]\n", position.x, position.y, position.z, normal.x, normal.y, normal.z, texcoord.x, texcoord.y);
	}
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
};

struct VertexPTNIndexKey
{
	int p;
	int t;
	int n;
	bool operator==(const VertexPTNIndexKey &other) const
	{
		return p == other.p &&
			   t == other.t &&
			   n == other.n;
	}
};
struct VertexIndexKeyHash
{
	std::size_t operator()(const VertexPTNIndexKey &key) const
	{
		// 計算每個整數的哈希值
		std::size_t h1 = std::hash<int>{}(key.p);
		std::size_t h2 = std::hash<int>{}(key.t);
		std::size_t h3 = std::hash<int>{}(key.n);

		// 使用位移和異或組合哈希值
		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

// TriangleMesh Declarations.
class TriangleMesh
{
public:
	// TriangleMesh Public Methods.
	TriangleMesh();
	~TriangleMesh();

	// Load the model from an *.OBJ file.
	bool LoadFromFile(const std::string &filePath, const bool normalized = true);

	// Create vertex and index buffers.
	void CreateBuffers();

	// Apply transform on CPU.
	void ApplyTransformCPU(const glm::mat4x4 &mvpMatrix);

	int GetNumVertices() const { return numVertices; }
	int GetNumTriangles() const { return numTriangles; }
	int GetNumIndices() const { return (int)vertexIndices.size(); }
	glm::vec3 GetObjCenter() const { return objCenter; }

	// 添加額外function
	GLuint GetVBOId() const { return vboId; }
	GLuint GetIBOId() const { return iboId; }

private:
	// TriangleMesh Private Methods.
	void PrintMeshInfo() const;
	// my function
	int findVertexPTNIndex(int p, int t, int n) const;

	// TriangleMesh Private Data.
	GLuint vboId;
	GLuint iboId;
	std::vector<VertexPTN> vertices;
	std::vector<unsigned int> vertexIndices;

	std::vector<glm::vec3> vertexPositions;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec2> vertexTexcoords;

	// my hash vector
	std::unordered_map<VertexPTNIndexKey, int, VertexIndexKeyHash> vertexMap; // 建立vertex map

	int numVertices;
	int numTriangles;
	glm::vec3 objCenter;
};

#endif
