#include "TriangleMesh.h"
#include <iostream>
#include <string>

// Desc: Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
	iboId = 0;
}

// Desc: Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	vertices.clear();
	vertexIndices.clear();
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &iboId);
}

// Desc: Load the geometry data of the model from file and normalize it.
bool TriangleMesh::LoadFromFile(const std::string &filePath, const bool normalized)
{
	// check if the file exists
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Error: Unable to open file " << filePath << std::endl;
		return false;
	}
	// if the file exists
	else
	{

		// update vertices and triangles
		std::string line;
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec2 texcoord = glm::vec2(0.0f, 0.0f);
		float px, py, pz, nx, ny, nz, uvx, uvy;

		while (std::getline(file, line))
		{
			std::cout << line << std::endl;
			if (line.substr(0, 2) == "v ")
			{
				numVertices++;
				std::istringstream(line.substr(2)) >> px >> py >> pz;
				position = glm::vec3(px, py, pz);
			}
			if (line.substr(0, 2) == "vt")
			{
				std::istringstream(line.substr(2)) >> uvx >> uvy;
				texcoord = glm::vec2(uvx, uvy);
			}
			if (line.substr(0, 2) == "vn")
			{
				std::istringstream(line.substr(2)) >> nx >> ny >> nz;
				normal = glm::vec3(nx, ny, nz);
			}
			if (line.substr(0, 2) == "f ")
			{
				//TODO:update triangles vector
				numTriangles++;
			}

			VertexPTN vertex(position, normal, texcoord);
			vertices.push_back(vertex);
		}


		// TODO:Add your code here.
		// ...

		if (normalized)
		{
			// Normalize the geometry data.
			// Add your code here.
			// ...
		}

		PrintMeshInfo();
		return true;
	}
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers()
{
	// Add your code here.
	// ...
}

// Desc: Apply transformation to all vertices (DON'T NEED TO TOUCH)
void TriangleMesh::ApplyTransformCPU(const glm::mat4x4 &mvpMatrix)
{
	for (int i = 0; i < numVertices; ++i)
	{
		glm::vec4 p = mvpMatrix * glm::vec4(vertices[i].position, 1.0f);
		if (p.w != 0.0f)
		{
			float inv = 1.0f / p.w;
			vertices[i].position.x = p.x * inv;
			vertices[i].position.y = p.y * inv;
			vertices[i].position.z = p.z * inv;
		}
	}
}

// Desc: Print mesh information.
void TriangleMesh::PrintMeshInfo() const
{
	std::cout << "[*] Mesh Information: " << std::endl;
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Center: (" << objCenter.x << " , " << objCenter.y << " , " << objCenter.z << ")" << std::endl;
}
