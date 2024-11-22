#include "trianglemesh.h"

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{

	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	vertices.clear();
	glDeleteBuffers(1, &vboId);
	// 因 iboId在subMesh成員中，需遞迴刪除
	for (auto &subMesh : subMeshes)
	{
		if (subMesh.iboId != 0)
			glDeleteBuffers(1, &subMesh.iboId);
	}
	subMeshes.clear();
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string &filePath, const bool normalized)
{
	/* 區域變數 */
	std::string line;
	std::ifstream objFile(filePath);
	SubMesh *subMesh = nullptr;
	std::string materialName;

	if (!objFile.is_open()) // 檢查檔案是否開啟
	{
		std::cerr << "Error: Unable to open file " << filePath << std::endl;
		return false;
	}

	/* 若文件存在，更新頂點和三角形buffer */
	while (std::getline(objFile, line))
	{
		std::string firstToken;
		std::istringstream iss(line); // 创建字符串流

		iss >> firstToken;

		if (firstToken == "v") // 更新頂點
		{
			glm::vec3 position;
			iss >> position.x >> position.y >> position.z;
			vertexPositions.push_back(position); // 將頂點加入vertex buffer
			numVertices++;
		}
		else if (firstToken == "vt")
		{
			glm::vec2 vertexTexcoord;
			iss >> vertexTexcoord.x >> vertexTexcoord.y;
			vertexTexcoords.push_back(vertexTexcoord); // 將texture加入texture buffer
		}
		else if (firstToken == "vn")
		{
			glm::vec3 vertexNormal;
			iss >> vertexNormal.x >> vertexNormal.y >> vertexNormal.z;
			vertexNormals.push_back(vertexNormal); // 將normal加入normal buffer
		}
		/* 以下開始為作業2的內容 */
		else if (firstToken == "mtllib") // 取得 mtl檔案 相對路徑
		{
			std::string mtlName, parentDirectory, mtlFilePath;
			size_t lastSlash = filePath.find_last_of("/\\"); // 查找最后一个斜杠的位置
			iss >> mtlName;
			if (lastSlash != std::string::npos)
				parentDirectory = filePath.substr(0, lastSlash + 1); // 返回目录部分，包括斜杠
			mtlFilePath = parentDirectory + mtlName;
			LoadFromMTLFile(mtlFilePath); // parse mtl檔案，建立mtlMap
		}
		else if (firstToken == "usemtl") // 建立 submesh
		{
			if (subMesh != nullptr)
			{
				subMeshes.push_back(*subMesh);
				delete subMesh;	   // 釋放內存，但還是指向相同記憶體位置
				subMesh = nullptr; // 將submesh指標設為空
			}
			iss >> materialName;
			subMesh = new SubMesh();
			subMesh->material = mtlMap[materialName]; // subMesh的material指向mtlMap中的定義的材質
		}

		/* 處理PTN */
		else if (firstToken == "f")
		{
			std::vector<unsigned int> polyIndices; // 先儲存PTN對應的index，若為多邊形則之後拆解(避免重複的PTN點要重複查詢index)
			while (!iss.eof())
			{ // 檢查是否達到文件結尾，若無
				int p, t, n;
				iss >> p, iss.ignore(1); // ignore '/'
				iss >> t, iss.ignore(1);
				iss >> n;

				VertexPTN newVertexPTN(vertexPositions[--p], vertexNormals[--n], vertexTexcoords[--t]);
				int index = findVertexPTNIndex(p, t, n); // 找PTN的index
				if (index != -1)						 // 找到PTN組合index，加入到vertexIndices中
				{
					polyIndices.push_back(index);
				}
				else // 若找不到，代表這個PTN組合是新的，需要加入到vertices中
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
			/* 處理多邊形indices(建立三角形，但是儲存於subMesh的vertexIndices) */
			for (size_t i = 1; i < polyIndices.size() - 1; i++)
			{
				subMesh->vertexIndices.push_back(polyIndices[0]);
				subMesh->vertexIndices.push_back(polyIndices[i]);
				subMesh->vertexIndices.push_back(polyIndices[i + 1]);
				numTriangles++;
			}
		}
	}

	/* 標準化頂點 */
	if (normalized)
	{
		glm::vec3 minVertex = glm::vec3(FLT_MAX);
		glm::vec3 maxVertex = glm::vec3(-FLT_MAX); // 與第一版不同：初始化最小值(負的最大值，就算值為負數，也會比這個大進而被取代)

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

		objCenter = 0.5f * (maxVertex + minVertex) * scaleFactor; // 計算中心並對其進行scale

		// 所有頂點減去中心點(移動到中心)
		for (auto &vertex : vertices)
		{
			vertex.position -= objCenter;
		}

		// 更新minVertex和maxVertex，以計算新的bounding box(objExtent)=1*1*1
		(minVertex *= scaleFactor, minVertex -= objCenter);
		(maxVertex *= scaleFactor, maxVertex -= objCenter);
		objExtent = maxVertex - minVertex;
	}

	return true;
}

/* 載入mtl檔案 */
bool TriangleMesh::LoadFromMTLFile(const std::string &mtlFilePath)
{
	// 區域變數
	float Ns;
	glm::vec3 Ka, Kd, Ks;
	std::string line;
	std::ifstream mtlFile(mtlFilePath);
	PhongMaterial *phongMaterial = nullptr;

	if (!mtlFile.is_open()) // 檢查檔案是否開啟
	{
		std::cerr << "Error: Unable to open file " << mtlFilePath << std::endl;
		return false;
	}

	// 讀取 mtl檔案，並依照mtlName和PhongMaterial 儲存在容器中(之後讀取f會需要映射)
	while (std::getline(mtlFile, line))
	{
		std::istringstream iss(line); // 創建字串流
		std::string firstToken;

		iss >> firstToken;
		if (firstToken == "newmtl")
		{
			std::string materialName;
			iss >> materialName;
			phongMaterial = new PhongMaterial();
			phongMaterial->SetName(materialName);
			mtlMap[materialName] = phongMaterial;
		}
		if (firstToken == "Ns")
		{
			iss >> Ns;
			phongMaterial->SetNs(Ns);
		}
		if (firstToken == "Ka")
		{
			iss >> Ka.x >> Ka.y >> Ka.z;
			phongMaterial->SetKa(Ka);
		}
		if (firstToken == "Kd")
		{
			iss >> Kd.x >> Kd.y >> Kd.z;
			phongMaterial->SetKd(Kd);
		}
		if (firstToken == "Ks")
		{
			iss >> Ks.x >> Ks.y >> Ks.z;
			phongMaterial->SetKs(Ks);
		}
	}

	// 印出所有mtl 名字和詳細資訊(for debug)
	for (auto &mtl : mtlMap)
	{
		printf("mtl name: %s\n", mtl.first.c_str());
		mtl.second->PrintInfo();
	}

	return true;
}

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

void TriangleMesh::CreateBuffers()
{
	// 產生vertex buffer
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPTN) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	// 對subMeshes產生iboId buffer
	for (auto &subMesh : subMeshes)
	{

		glGenBuffers(1, &subMesh.iboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * subMesh.vertexIndices.size(), &subMesh.vertexIndices[0], GL_STATIC_DRAW);
	}
}

void TriangleMesh::Render(PhongShadingDemoShaderProg *shader) // ref sphere.cpp Render()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	// 綁定 position 和 normal
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void *)offsetof(VertexPTN, position)); // pos offset is 0
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void *)offsetof(VertexPTN, normal)); // pos offset is 0

	// 對iboId 對遞迴處理(shader 使用uniform 綁定參數)
	for (auto &subMesh : subMeshes)
	{
		if (subMesh.material)
		{
			glUniform3fv(shader->GetLocKa(), 1, glm::value_ptr(subMesh.material->GetKa()));
			glUniform3fv(shader->GetLocKd(), 1, glm::value_ptr(subMesh.material->GetKd()));
			glUniform3fv(shader->GetLocKs(), 1, glm::value_ptr(subMesh.material->GetKs()));
			glUniform1f(shader->GetLocNs(), subMesh.material->GetNs());
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh.iboId);
		glDrawElements(GL_TRIANGLES, (GLsizei)(subMesh.vertexIndices.size()), GL_UNSIGNED_INT, 0);

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
