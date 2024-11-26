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

			// 處理PTN
			if (line.substr(0, 2) == "f ")
			{
				std::vector<unsigned int> polyIndices;	// 先儲存PTN對應的index，若為多邊形則之後拆解(避免重複的PTN點要重複查詢index)
				std::istringstream iss(line.substr(2)); // 去掉'f'前綴，並不間斷讀取
				std::string tokenPTN;
				while (iss >> tokenPTN)
				{
					int p, t, n;
					if (sscanf(tokenPTN.c_str(), "%d/%d/%d", &p, &t, &n) == 3) // 檢查是否成功parse
					{
						VertexPTN newVertexPTN(vertexPositions[--p], vertexNormals[--n], vertexTexcoords[--t]);
						int index = findVertexPTNIndex(p, t, n); // 找PTN的index
						if (index != -1)									 // 找到PTN組合index，加入到vertexIndices中
						{
							polyIndices.push_back(index);
						}
						else // 若找不到，代表這個PTN組合是新的，需要加入到vertices中
						{
							polyIndices.push_back(numVertices);
							//將PTN hash出的組合加入到vertexMap中
							VertexPTNIndexKey key = {p, t, n};
							vertexMap[key] = numVertices;
							//將PTN組合加入到vertex中
							vertices.push_back(newVertexPTN);
							numVertices++;
						}
					}
					else
						printf("Parse error!\n");
				}
				// 處理多邊形indices
				for (int i = 1; i < polyIndices.size() - 1; i++)
				{
					vertexIndices.push_back(polyIndices[0]);
					vertexIndices.push_back(polyIndices[i]);
					vertexIndices.push_back(polyIndices[i + 1]);
				}
			}
		}

		// 標準化頂點
		if (normalized)
		{
			glm::vec3 minVertex = glm::vec3(FLT_MAX);
			glm::vec3 maxVertex = glm::vec3(-FLT_MIN);

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
int TriangleMesh::findVertexPTNIndex(int p, int t, int n) const
{
	VertexPTNIndexKey key = {p, t, n};
	auto it = vertexMap.find(key);
	if (it != vertexMap.end())
	{
		return it->second;
	}
	return -1;
	// for (int i = 0; i < vertices.size(); i++)
	// {
	// 	if (vertices[i].isEqual(targetVertexPTN))
	// 	{
	// 		return i;
	// 	}
	// }
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
