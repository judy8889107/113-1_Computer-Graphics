#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "headers.h"
#include "material.h"

// 我的header
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

// SubMesh Declarations.
struct SubMesh
{
	SubMesh()
	{
		material = nullptr;
		iboId = 0;
	}
	PhongMaterial *material;
	GLuint iboId;
	std::vector<unsigned int> vertexIndices;
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

	// Show model information.
	void ShowInfo();

	// -------------------------------------------------------
	// Feel free to add your methods or data here.
	void CreateBuffers(); // 建立頂點和索引緩衝區。
	void Render(PhongMaterial *shader);
	// -------------------------------------------------------

	

	int GetNumVertices() const { return numVertices; }
	int GetNumTriangles() const { return numTriangles; }
	int GetNumSubMeshes() const { return (int)subMeshes.size(); }

	glm::vec3 GetObjCenter() const { return objCenter; }
	glm::vec3 GetObjExtent() const { return objExtent; }

	// 添加額外function
	GLuint GetVBOId() const { return vboId; }
	// GLuint GetIBOId() const { return iboId; }

private:
	// -------------------------------------------------------
	// Feel free to add your methods or data here.
	/* 我的定義 */
	std::unordered_map<VertexPTNIndexKey, int, VertexIndexKeyHash> vertexMap; // 建立vertex map
	std::unordered_map<std::string, PhongMaterial *> mtlMap;				  // 建立mtl map
	std::vector<SubMesh> subMeshes;											  // subMeshes vector用於儲存使用不同材質的子mesh
	int findVertexPTNIndex(int p, int t, int n) const;						  // hash find ptn index
	bool LoadFromMTLFile(const std::string &filePath); // parse mtl file
	// -------------------------------------------------------
	// TriangleMesh Private Methods.
	// void PrintMeshInfo() const;

	// TriangleMesh Private Data.
	GLuint vboId;

	std::vector<VertexPTN> vertices; // 所有的vertex
	
	// For supporting multiple materials per object, move to SubMesh.
	// GLuint iboId;
	// std::vector<unsigned int> vertexIndices;

	// 我的vector
	std::vector<glm::vec3> vertexPositions;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec2> vertexTexcoords;

	int numVertices;
	int numTriangles;
	glm::vec3 objCenter;
	glm::vec3 objExtent;
};

#endif
