#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION // add this to exactly 1 of your C++ files
#include <tiny_obj_loader.h>

namespace Geometry
{
namespace fs = std::filesystem;

bool load2DGeometry(const fs::path& path, std::vector<float>& vertexData, std::vector<uint16_t>& indexData) 
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    vertexData.clear();
    indexData.clear();

    enum class Section {
        None,
        Points,
        Indices,
    };
    Section currentSection = Section::None;

    float value;
    uint16_t index;
    std::string line;
    while (!file.eof()) {
        getline(file, line);
        
        // overcome the `CRLF` problem
            if (!line.empty() && line.back() == '\r') {
              line.pop_back();
            }
        
        if (line == "[points]") {
            currentSection = Section::Points;
        }
        else if (line == "[indices]") {
            currentSection = Section::Indices;
        }
        else if (line[0] == '#' || line.empty()) {
            // Do nothing, this is a comment
        }
        else if (currentSection == Section::Points) {
            std::istringstream iss(line);
            // Get x, y, r, g, b
            for (int i = 0; i < 5; ++i) {
                iss >> value;
                vertexData.push_back(value);
            }
        }
        else if (currentSection == Section::Indices) {
            std::istringstream iss(line);
            // Get corners #0 #1 and #2
            for (int i = 0; i < 3; ++i) {
                iss >> index;
                indexData.push_back(index);
            }
        }
    }
    return true;
}

bool load3DGeometry(const fs::path& path, std::vector<float>& pointData, std::vector<uint16_t>& indexData, int dimensions) {
	std::ifstream file(path);
	if (!file.is_open()) {
		return false;
	}

	pointData.clear();
	indexData.clear();

	enum class Section {
		None,
		Points,
		Indices,
	};
	Section currentSection = Section::None;

	float value;
	uint16_t index;
	std::string line;
	while (!file.eof()) {
		getline(file, line);
		if (line == "[points]") {
			currentSection = Section::Points;
		}
		else if (line == "[indices]") {
			currentSection = Section::Indices;
		}
		else if (line[0] == '#' || line.empty()) {
			// Do nothing, this is a comment
		}
		else if (currentSection == Section::Points) {
			std::istringstream iss(line);
			// Get x, y, r, g, b
			for (int i = 0; i < dimensions + 3; ++i) {
				iss >> value;
				pointData.push_back(value);
			}
		}
		else if (currentSection == Section::Indices) {
			std::istringstream iss(line);
			// Get corners #0 #1 and #2
			for (int i = 0; i < 3; ++i) {
				iss >> index;
				indexData.push_back(index);
			}
		}
	}
	return true;
}

template<typename T>
bool loadGeometryFromObj(const fs::path& path, std::vector<T>& vertexData) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	// Call the core loading procedure of TinyOBJLoader
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

	// Check errors
	if (!warn.empty()) {
		std::cout << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	// Filling in vertexData:
	vertexData.clear();
	for (const auto& shape : shapes) {
		size_t offset = vertexData.size();
		vertexData.resize(offset + shape.mesh.indices.size());

		for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
			const tinyobj::index_t& idx = shape.mesh.indices[i];

			vertexData[offset + i].position = {
				attrib.vertices[3 * idx.vertex_index + 0],
				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
				attrib.vertices[3 * idx.vertex_index + 1]
			};

			// Also apply the transform to normals!!
			vertexData[offset + i].normal = {
				attrib.normals[3 * idx.normal_index + 0],
				-attrib.normals[3 * idx.normal_index + 2],
				attrib.normals[3 * idx.normal_index + 1]
			};

			vertexData[offset + i].color = {
				attrib.colors[3 * idx.vertex_index + 0],
				attrib.colors[3 * idx.vertex_index + 1],
				attrib.colors[3 * idx.vertex_index + 2]
			};
		}
	}

	return true;
}

template<typename T1>
glm::mat3x3 computeTBN(const T1 corners[3], const glm::vec3& expectedN) {
	// What we call e in the figure
	glm::vec3 ePos1 = corners[1].position - corners[0].position;
	glm::vec3 ePos2 = corners[2].position - corners[0].position;

	// What we call \bar e in the figure
	glm::vec2 eUV1 = corners[1].uv - corners[0].uv;
	glm::vec2 eUV2 = corners[2].uv - corners[0].uv;

	glm::vec3 T = normalize(ePos1 * eUV2.y - ePos2 * eUV1.y);
	glm::vec3 B = normalize(ePos2 * eUV1.x - ePos1 * eUV2.x);
	glm::vec3 N = cross(T, B);

	// Fix overall orientation
	if (dot(N, expectedN) < 0.0) {
		T = -T;
		B = -B;
		N = -N;
	}

	// Ortho-normalize the (T, B, expectedN) frame
	// a. "Remove" the part of T that is along expected N
	N = expectedN;
	T = normalize(T - dot(T, N) * N);
	// b. Recompute B from N and T
	B = cross(N, T);

	return glm::mat3x3(T, B, N);
}

template<typename T>
void populateTextureFrameAttributes(std::vector<T>& vertexData) {
	size_t triangleCount = vertexData.size() / 3;
	// We compute the local texture frame per triangle
	for (int t = 0; t < triangleCount; ++t) {
		T* v = &vertexData[3 * t];

		for (int k = 0; k < 3; ++k) {
			glm::mat3x3 TBN = computeTBN<T>(v, v[k].normal);
			v[k].tangent = TBN[0];
			v[k].bitangent = TBN[1];
		}
	}
}

template<typename T>
bool loadGeometryFromObjWithUV(const fs::path& path, std::vector<T>& vertexData) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	// Call the core loading procedure of TinyOBJLoader
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

	// Check errors
	if (!warn.empty()) {
		std::cout << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	// Filling in vertexData:
	vertexData.clear();
	for (const auto& shape : shapes) {
		size_t offset = vertexData.size();
		vertexData.resize(offset + shape.mesh.indices.size());

		for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
			const tinyobj::index_t& idx = shape.mesh.indices[i];

			vertexData[offset + i].position = {
				attrib.vertices[3 * idx.vertex_index + 0],
				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
				attrib.vertices[3 * idx.vertex_index + 1]
			};

			vertexData[offset + i].normal = {
				attrib.normals[3 * idx.normal_index + 0],
				-attrib.normals[3 * idx.normal_index + 2],
				attrib.normals[3 * idx.normal_index + 1]
			};

			vertexData[offset + i].color = {
				attrib.colors[3 * idx.vertex_index + 0],
				attrib.colors[3 * idx.vertex_index + 1],
				attrib.colors[3 * idx.vertex_index + 2]
			};

			vertexData[offset + i].uv = {
				attrib.texcoords[2 * idx.texcoord_index + 0],
				1 - attrib.texcoords[2 * idx.texcoord_index + 1]
			};
		}
	}

	return true;
}

}