#include "trianglemesh.h"

// my header
#include <fstream>
#include "material.h"

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	// -------------------------------------------------------
	// TODO:Add your initialization code here.
	// -------------------------------------------------------
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.f, 0.0f, 0.0f);
	vboId = 0;
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	// -------------------------------------------------------
	// TODO:Add your release code here.
	// -------------------------------------------------------
	vertices.clear();
	// Delete vbo
	glDeleteBuffers(1, &vboId);

	// 迭代 subMeshes
	for (auto &subMesh : subMeshes)
	{
		// Delete the IBO for each subMesh.
		if (subMesh.iboId != 0)
		{
			glDeleteBuffers(1, &subMesh.iboId);
		}
	}
	subMeshes.clear();
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string &filePath, const bool normalized)
{
	// ---------------------------------------------------------------------------
	// TODO:Add your implementation here (HW1 + read *.MTL).
	// ---------------------------------------------------------------------------

	// check file can be open
	std::ifstream objFile(filePath);
	if (!objFile.is_open())
	{
		std::cerr << "[ERROR] Failed to open OBJ file: " << filePath << std::endl;
		return false;
	}

	// read .obj file
	std::string line;
	SubMesh *subMesh = nullptr;

	while (std::getline(objFile, line))
	{
		std::istringstream iss(line);
		std::string firstToken;
		iss >> firstToken;

		if (firstToken == "v")
		{
			glm::vec3 position;
			iss >> position.x >> position.y >> position.z;
			vertexPositions.push_back(position);
		}
		else if (firstToken == "vt")
		{
			glm::vec2 texcoord;
			iss >> texcoord.x >> texcoord.y;
			vertexTexcoords.push_back(texcoord);
		}
		else if (firstToken == "vn")
		{
			glm::vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			vertexNormals.push_back(normal);
		}
		else if (firstToken == "mtllib") // load .mtl file
		{
			size_t dotPos = filePath.find_last_of('.');
			std::string mtlFile = filePath.substr(0, dotPos) + ".mtl";
			if (!LoadFromMTLFile(mtlFile))
			{
				std::cerr << "[ERROR] Failed to parse MTL file: " << mtlFile << std::endl;
			}
		}
		// create submesh
		else if (firstToken == "usemtl")
		{
			std::string mtlName;
			iss >> mtlName;
			if (subMesh != nullptr)
			{
				subMeshes.push_back(*subMesh);
				delete subMesh;
				subMesh = nullptr;
			}
			subMesh = new SubMesh();
			subMesh->material = mtlMap[mtlName];
		}
		else if (firstToken == "f")
		{
			std::vector<unsigned int> polyIndices; // 先儲存PTN對應的index，若為多邊形則之後拆解(避免重複的PTN點要重複查詢index)

			while (!iss.eof())
			{
				int p, t, n;
				iss >> p;
				iss.ignore(1);
				iss >> t;
				iss.ignore(1);
				iss >> n;

				VertexPTN newVertexPTN(vertexPositions[p - 1], vertexNormals[n - 1], vertexTexcoords[t - 1]);
				int index = findVertexPTNIndex(p, t, n); // 找PTN的index
				if (index != -1)						 // 找到PTN組合index，加入到vertexIndices中
				{
					polyIndices.push_back(index);
				}
				else
				{
					polyIndices.push_back(numVertices);
					// 將PTN hash出的組合加入到vertexMap中
					VertexPTNIndexKey key = {p, t, n};
					vertexMap[key] = numVertices;
					// 將PTN組合加入到vertex中
					vertices.push_back(newVertexPTN);
					numVertices++;
				}
			}
			// 處理多邊形indices
			for (int i = 1; i < polyIndices.size() - 1; i++)
			{
				subMesh->vertexIndices.push_back(polyIndices[0]);
				subMesh->vertexIndices.push_back(polyIndices[i]);
				subMesh->vertexIndices.push_back(polyIndices[i + 1]);
				numTriangles += 1;
			}
		}
	}

	// 將最後一個 subMesh 加入到subMeshes中
	if (subMesh != nullptr)
	{
		subMeshes.push_back(*subMesh);
		delete subMesh;
		subMesh = nullptr;
	}

	// Normalize the geometry data.
	if (normalized)
	{
		// -----------------------------------------------------------------------
		// TODO:Add your normalization code here (HW1).
		// -----------------------------------------------------------------------
		glm::vec3 minVertex = glm::vec3(FLT_MAX);
		glm::vec3 maxVertex = glm::vec3(-FLT_MAX); // for 有負號

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
		// 重新計算bounding box尺寸
		minVertex *= scaleFactor;
		minVertex -= objCenter;

		maxVertex *= scaleFactor;
		maxVertex -= objCenter;
		objExtent = maxVertex - minVertex;
	}
	return true;
}

// Load the material data from an MTL file.
bool TriangleMesh::LoadFromMTLFile(const std::string &filePath)
{
	printf("[Debug] Load mtlFile: %s\n", filePath.c_str());
	// check file can be open
	std::ifstream mtlFile(filePath);
	if (!mtlFile.is_open())
	{
		std::cerr << "[ERROR] Failed to open OBJ file: " << filePath << std::endl;
		return false;
	}

	// read .obj file
	std::string line;
	PhongMaterial *material = nullptr;
	while (std::getline(mtlFile, line))
	{
		std::istringstream iss(line);
		std::string firstToken;
		iss >> firstToken;

		if (firstToken == "newmtl")
		{
			std::string mtlName;
			iss >> mtlName;
			material = new PhongMaterial();
			material->SetName(mtlName);
			mtlMap[mtlName] = material;
		}
		else if (firstToken == "Ns")
		{
			float Ns;
			iss >> Ns;
			material->SetNs(Ns);
		}
		else if (firstToken == "Ka")
		{
			glm::vec3 Ka;
			iss >> Ka.x >> Ka.y >> Ka.z;
			material->SetKa(Ka);
		}
		else if (firstToken == "Kd")
		{
			glm::vec3 Kd;
			iss >> Kd.x >> Kd.y >> Kd.z;
			material->SetKd(Kd);
		}
		else if (firstToken == "Ks")
		{
			glm::vec3 Ks;
			iss >> Ks.x >> Ks.y >> Ks.z;
			material->SetKs(Ks);
		}
	}

	// for debug
	printf("[Debug] mtlMap size: %d\n", mtlMap.size());
	for (auto it = mtlMap.begin(); it != mtlMap.end(); it++)
	{
		printf("MtlName: %s\n", it->first.c_str());
		printf("Ns: %f\n", it->second->GetNs());
		printf("Ka: %f %f %f\n", it->second->GetKa().x, it->second->GetKa().y, it->second->GetKa().z);
		printf("Kd: %f %f %f\n", it->second->GetKd().x, it->second->GetKd().y, it->second->GetKd().z);
		printf("Ks: %f %f %f\n", it->second->GetKs().x, it->second->GetKs().y, it->second->GetKs().z);
	}

	return true;
}

// Find vertex ptn index
int TriangleMesh::findVertexPTNIndex(int p, int t, int n) const
{
	VertexPTNIndexKey key = {p, t, n};
	auto it = vertexMap.find(key);
	if (it != vertexMap.end())
	{
		return it->second;
	}
	return -1;
}

// create buffer
void TriangleMesh::CreateBuffer()
{
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPTN) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// Create index buffer.
	for (auto &subMesh : subMeshes)
	{
		glGenBuffers(1, &subMesh.iboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * subMesh.vertexIndices.size(), &subMesh.vertexIndices[0], GL_STATIC_DRAW);
	}
}

// render
void TriangleMesh::Render(PhongShadingDemoShaderProg *shader)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void *)offsetof(VertexPTN, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void *)offsetof(VertexPTN, normal));

	for (const auto &subMesh : subMeshes)
	{

		if (subMesh.material)
		{
			glUniform3fv(shader->GetLocKa(), 1, glm::value_ptr(subMesh.material->GetKa()));
			glUniform3fv(shader->GetLocKd(), 1, glm::value_ptr(subMesh.material->GetKd()));
			glUniform3fv(shader->GetLocKs(), 1, glm::value_ptr(subMesh.material->GetKs()));
			glUniform1f(shader->GetLocNs(), subMesh.material->GetNs());
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.iboId);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(subMesh.vertexIndices.size()), GL_UNSIGNED_INT, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// Show model information.
void TriangleMesh::ShowInfo()
{
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
	for (unsigned int i = 0; i < subMeshes.size(); ++i)
	{
		const SubMesh &g = subMeshes[i];
		std::cout << "SubMesh " << i << " with material: " << g.material->GetName() << std::endl;
		std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3 << std::endl;
	}
	std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", " << objCenter.z << std::endl;
	std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x " << objExtent.z << std::endl;
}
