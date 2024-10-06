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
	// 檢查文件是否存在
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Error: Unable to open file " << filePath << std::endl;
		return false;
	}
	// 如果文件存在
	else
	{

		// 更新頂點和三角形buffer
		std::string line;
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec2 texcoord = glm::vec2(0.0f, 0.0f);
		float px, py, pz, nx, ny, nz, uvx, uvy;
		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float minZ = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();
		float maxZ = std::numeric_limits<float>::lowest();

		while (std::getline(file, line))
		{
			std::cout << line << std::endl;
			// 更新頂點
			if (line.substr(0, 2) == "v ")
			{
				numVertices++;
				std::istringstream(line.substr(2)) >> px >> py >> pz;
				position = glm::vec3(px, py, pz);

				// 更新最小值
				if (px < minX)
					minX = px;
				if (py < minY)
					minY = py;
				if (pz < minZ)
					minZ = pz;
				// 更新最大值
				if (px > maxX)
					maxX = px;
				if (py > maxY)
					maxY = py;
				if (pz > maxZ)
					maxZ = pz;
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

			VertexPTN vertex(position, normal, texcoord);
			vertices.push_back(vertex);

			// 更新三角形
			if (line.substr(0, 2) == "f ")
			{

				std::vector<std::string> tokens;
				std::stringstream ss(line.substr(2));
				std::string tok;

				// 以單一空格分割
				while (std::getline(ss, tok, ' '))
				{
					tokens.push_back(tok);
				}

				// 排列組合成三角形
				if (tokens.size() > 3)
				{

					for (int j = 1; j < tokens.size() - 1; j++)
					{
						vertexIndices.push_back(std::stoi(tokens[0].substr(0, 1)));
						vertexIndices.push_back(std::stoi(tokens[j].substr(0, 1)));
						vertexIndices.push_back(std::stoi(tokens[j + 1].substr(0, 1)));
						numTriangles++;
					}
				}
				else
				{
					numTriangles++;
				}
			}
		}

		// 更新中心點
		objCenter = glm::vec3((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f);

		if (normalized)
		{
			//正規化所有position
			float rangeX = maxX - minX;
			float rangeY = maxY - minY;
			float rangeZ = maxZ - minZ;

			for (auto &vertex : vertices)
			{
				vertex.position = glm::vec3((vertex.position.x - minX) / rangeX, (vertex.position.y - minY) / rangeY, (vertex.position.z - minZ) / rangeZ);
			}
		}


		PrintMeshInfo();
		return true;
	}
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers()
{
	// 產生vertex buffer
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVertices, &vertices[0], GL_STATIC_DRAW);
	//產生 index buffer(幾個三角形*3個頂點)
	glGenBuffers(1, &iboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numTriangles * 3, &vertexIndices[0] , GL_STATIC_DRAW);
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
