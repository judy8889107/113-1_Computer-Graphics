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
				glm::vec3 position;
				sscanf(line.c_str(), "v %f %f %f", &position.x, &position.y, &position.z);

				vertexPositions.push_back(position); // 將頂點加入vertex buffer
				// numVertices++;
			}
			else if (line.substr(0, 2) == "vt")
			{
				glm::vec2 vertexTexcoord;
				sscanf(line.c_str(), "vt %f %f", &vertexTexcoord.x, &vertexTexcoord.y);
				vertexTexcoords.push_back(vertexTexcoord); // 將texture加入texture buffer
			}
			else if (line.substr(0, 2) == "vn")
			{
				glm::vec3 vertexNormal;
				sscanf(line.c_str(), "vn %f %f %f", &vertexNormal.x, &vertexNormal.y, &vertexNormal.z);
				vertexNormals.push_back(vertexNormal); // 將normal加入normal buffer
			}

			// 更新三角形index
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

				// 排列組合成三角形，並紀錄index和計算數量
				if (tokens.size() > 3)
				{

					for (int j = 1; j < tokens.size() - 1; j++)
					{
						int p1, t1, n1, p2, t2, n2, p3, t3, n3;
						sscanf(tokens[0].c_str(), "%d/%d/%d", &p1, &t1, &n1);
						sscanf(tokens[j].c_str(), "%d/%d/%d", &p2, &t2, &n2);
						sscanf(tokens[j + 1].c_str(), "%d/%d/%d", &p3, &t3, &n3);

						VertexPTN vertexPTN1(vertexPositions[p1 - 1], vertexNormals[n1 - 1], vertexTexcoords[t1 - 1]);
						VertexPTN vertexPTN2(vertexPositions[p2 - 1], vertexNormals[n2 - 1], vertexTexcoords[t2 - 1]);
						VertexPTN vertexPTN3(vertexPositions[p3 - 1], vertexNormals[n3 - 1], vertexTexcoords[t3 - 1]);

						// 加入PTN buffer
						vertices.push_back(vertexPTN1);
						vertices.push_back(vertexPTN2);
						vertices.push_back(vertexPTN3);

						// 加入三角形index
						vertexIndices.push_back(0);
						vertexIndices.push_back(j);
						vertexIndices.push_back(j + 1);

						printf("[1] %d/%d/%d\n", p1, t1, n1);
						printf("[2] %d/%d/%d\n", p2, t2, n2);
						printf("[3] %d/%d/%d\n", p3, t3, n3);
						numTriangles++;
					}
				}
				else
				{

					for (auto &token : tokens)
					{
						int p, t, n;
						sscanf(token.c_str(), "%d/%d/%d", &p, &t, &n);
						VertexPTN vertexPTN(vertexPositions[p - 1], vertexNormals[n - 1], vertexTexcoords[t - 1]);
						printf("add index: %d\n", numVertices);
						printf("[non poly-1] %d/%d/%d\n", p, t, n);
						vertexPTN.print();
						vertices.push_back(vertexPTN);

						// 加入三角形index
						vertexIndices.push_back(numVertices);
						numVertices++;
					}
					// 若無需排列組合則直接計算數量即可
					numTriangles++;
				}
			}
		}

		// 更新中心點
		// objCenter = glm::vec3((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f);

		if (normalized)
		{
			// // TODO:正規化所有position
			// float rangeX = maxX - minX;
			// float rangeY = maxY - minY;
			// float rangeZ = maxZ - minZ;

			// for (auto &vertex : vertices)
			// {
			// 	vertex.position = glm::vec3((vertex.position.x - minX) / rangeX, (vertex.position.y - minY) / rangeY, (vertex.position.z - minZ) / rangeZ);
			// }
		}

		std::cout << "size of vertices: " << vertices.size() << std::endl;
		std::cout << "size of vertexIndices: " << vertexIndices.size() << std::endl;

		// 印出vertex vector和vertices vector
		// for (auto &vertex : vertices)
		// {
		// 	vertex.print();
		// }

		// for (int i = 0; i < vertexIndices.size(); i += 3)
		// {
		// 	printf("verticeIndices[%d]: %d %d %d\n", i, vertexIndices[i], vertexIndices[i + 1], vertexIndices[i + 2]);
		// }

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPTN) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &iboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * vertexIndices.size(), &vertexIndices[0], GL_STATIC_DRAW);
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
