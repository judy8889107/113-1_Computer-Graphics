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

		while (std::getline(file, line))
		{
			// printf("%s\n", line.c_str());
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
				std::vector<unsigned int> polyIndices; //先儲存PTN對應的index，若為多邊形則之後拆解
				std::istringstream iss(line.substr(2)); //去掉'f'前綴，並不間斷讀取
				std::vector<std::string> tokens;
				std::stringstream ss(line.substr(2));
				std::string tok;

				// 以單一空格分割
				while (std::getline(ss, tok, ' '))
				{
					tokens.push_back(tok);
				}

				// TODO:對於超過 3 個頂點的多邊形
				if (tokens.size() > 3)
				{
					int p, t, n;
					sscanf(tokens[0].c_str(), "%d/%d/%d", &p, &t, &n); // 固定頂點
					VertexPTN fixedVertexPTN(vertexPositions[p - 1], vertexNormals[n - 1], vertexTexcoords[t - 1]);
					int fixedIndex = findVertexPTNIndex(fixedVertexPTN);
					if (fixedIndex == -1) // 若找不到固定頂點，則加入固定頂點
					{
						vertices.emplace_back(fixedVertexPTN);
						// fixedVertexPTN.print();
						numVertices++;
					}
					for (int j = 1; j < tokens.size() - 1; j++) // 剩下頂點兩兩一組，和固定頂點形成三角形
					{

						int p1, t1, n1, p2, t2, n2;
						sscanf(tokens[j].c_str(), "%d/%d/%d", &p1, &t1, &n1);
						sscanf(tokens[j + 1].c_str(), "%d/%d/%d", &p2, &t2, &n2);

						VertexPTN newVertexPTN1(vertexPositions[p1 - 1], vertexNormals[n1 - 1], vertexTexcoords[t1 - 1]);
						VertexPTN newVertexPTN2(vertexPositions[p2 - 1], vertexNormals[n2 - 1], vertexTexcoords[t2 - 1]);

						// TODO: 檢查兩個頂點是否存在，若不存在則加入
						int index1 = findVertexPTNIndex(newVertexPTN1);
						int index2 = findVertexPTNIndex(newVertexPTN2);

						// MAKE A TIRANGLE
						vertexIndices.push_back(fixedIndex); // 加入固定頂點index
						if (index1 == -1)
						{
							vertexIndices.emplace_back(numVertices);
							vertices.emplace_back(newVertexPTN1);
							// newVertexPTN1.print();
							numVertices++;
						}
						else
						{
							vertexIndices.emplace_back(index1);
						}

						if (index2 == -1)
						{
							vertexIndices.emplace_back(numVertices);
							vertices.emplace_back(newVertexPTN2);
							// newVertexPTN2.print();
							numVertices++;
						}
						else
						{
							vertexIndices.emplace_back(index2);
						}
						numTriangles++;
					}
				}
				else // 一般三角形
				{

					numTriangles++; // 計算三角形數量
					for (auto &token : tokens)
					{
						int p, t, n;
						sscanf(token.c_str(), "%d/%d/%d", &p, &t, &n);
						VertexPTN newVertexPTN(vertexPositions[p - 1], vertexNormals[n - 1], vertexTexcoords[t - 1]);
						int index = findVertexPTNIndex(newVertexPTN); // 找PTN的index
						// printf("index: %d\n", index);
						if (index != -1) // 找到PTN組合index，加入到vertexIndices中
						{
							vertexIndices.push_back(index);
						}
						else // 若找不到，代表這個PTN組合是新的，需要加入到vertices中
						{
							vertexIndices.push_back(numVertices);
							vertices.push_back(newVertexPTN);
							// newVertexPTN.print();
							numVertices++;
						}
					}
				}
			}
		}

		// 標準化頂點
		if (normalized)
		{
			glm::vec3 minVertex = glm::vec3(FLT_MAX);
			glm::vec3 maxVertex = glm::vec3(FLT_MIN);

			// 找最小和最大座標
			for (const auto &vertex : vertices)
			{
				minVertex.x = std::min(minVertex.x, vertex.position.x);
				minVertex.y = std::min(minVertex.y, vertex.position.y);
				minVertex.z = std::min(minVertex.z, vertex.position.z);

				maxVertex.x = std::max(maxVertex.x, vertex.position.x);
				maxVertex.y = std::max(maxVertex.y, vertex.position.y);
				maxVertex.z = std::max(maxVertex.z, vertex.position.z);
			}

			// bounding box 尺寸
			glm::vec3 BoundingboxDimension = maxVertex - minVertex;

			// 計算縮放比例 --> 讓 最大邊長 == 1
			float scaleFactor = 1.0f / glm::max(glm::max(BoundingboxDimension.x, BoundingboxDimension.y), BoundingboxDimension.z);

			// Scale All Vertices
			for (auto &vertex : vertices)
			{
				vertex.position *= scaleFactor; // scale
			}

			// Calculate Center
			objCenter = 0.5f * (maxVertex + minVertex) * scaleFactor; // 計算中心並對其進行scale

			// 所有頂點減去中心點(移動到中心)
			for (auto &vertex : vertices)
			{
				vertex.position -= objCenter;
			}
		}

		PrintMeshInfo();
		return true;
	}
}

// 找是否vertices中已有重複的PTN組合，若有則回傳其index，否則回傳-1
int TriangleMesh::findVertexPTNIndex(VertexPTN targetVertexPTN) const
{

	for (int i = 0; i < vertices.size(); i++)
	{
		if (vertices[i].isEqual(targetVertexPTN))
		{
			return i;
		}
	}

	return -1;
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
