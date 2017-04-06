#include <iostream>

#include "libplyxx.h"

bool areClose(double a, double b)
{
	const double EPSILON = 1.0e-1;
	return abs(a - b) < EPSILON;
}

struct Vertex
{
	Vertex(double x, double y, double z) : x(x), y(y), z(z) {};
	bool operator==(const Vertex& other) const { return areClose(x, other.x) && areClose(y, other.y) && areClose(z, other.z); };
	double x, y, z;
};

struct Mesh
{
	typedef unsigned int VertexIndex;
	typedef std::array<VertexIndex, 3> TriangleIndices;
	typedef std::vector<Vertex> VertexList;
	typedef std::vector<TriangleIndices> TriangleIndicesList;

	Mesh(const VertexList& vertices, const TriangleIndicesList& triangles) : vertices(vertices), triangles(triangles) {};
	Mesh(VertexList&& vertices, TriangleIndicesList&& triangles) : vertices(std::move(vertices)), triangles(std::move(triangles)) {};

	VertexList vertices;
	TriangleIndicesList triangles;
};

void readply(std::wstring filename, Mesh::VertexList& vertices, Mesh::TriangleIndicesList& triangles)
{
	libply::File file(filename);
	const auto& definitions = file.definitions();

	const auto vertexDefinition = definitions.at(0);
	const size_t vertexCount = vertexDefinition.size;
	vertices.reserve(vertexCount);
	libply::ElementReadCallback vertexCallback = [&vertices](libply::ElementBuffer& e)
		{
			vertices.emplace_back(e[0], e[1], e[2]);
		};

	const auto triangleDefinition = definitions.at(1);
	const size_t triangleCount = triangleDefinition.size;
	triangles.reserve(triangleCount);
	libply::ElementReadCallback triangleCallback = [&triangles](libply::ElementBuffer& e)
	{
		triangles.emplace_back(std::move(Mesh::TriangleIndices{ e[0], e[1], e[2]}));
	};

	file.setElementReadCallback("vertex", vertexCallback);
	file.setElementReadCallback("face", triangleCallback);
	file.read();
}

void writeply(std::wstring filename, libply::ElementsDefinition& definitions, Mesh::VertexList& vertices, Mesh::TriangleIndicesList& triangles, libply::File::Format format)
{
	libply::FileOut file(filename, format);
	file.setElementsDefinition(definitions);
	
	libply::ElementWriteCallback vertexCallback = [&vertices](libply::ElementBuffer& e, size_t index)
		{
			const auto& v = vertices[index];
			e[0] = v.x;
			e[1] = v.y;
			e[2] = v.z;
		};
	libply::ElementWriteCallback triangleCallback = [&triangles](libply::ElementBuffer& e, size_t index)
		{
			e.reset(3);
			const auto& t = triangles[index];
			e[0] = t[0];
			e[1] = t[1];
			e[2] = t[2];
		};

	file.setElementWriteCallback("vertex", vertexCallback);
	file.setElementWriteCallback("face", triangleCallback);
	file.write();
}

bool compare_vertices(const Mesh::VertexList& left, const Mesh::VertexList& right)
{
	if (left.size() != right.size())
	{
		std::cout << "Length mismatch" << std::endl;
		return false;
	}

	int errors = 0;
	for (unsigned int i = 0; i < left.size(); ++i)
	{
		if (!(left[i] == right[i]))
		{
			std::cout << "vertex " << i << " is different" << std::endl;
			++errors;
		}
	}
	return errors == 0;
}

bool compare_triangles(const Mesh::TriangleIndicesList& left, const Mesh::TriangleIndicesList& right)
{
	if (left.size() != right.size())
	{
		std::cout << "Length mismatch" << std::endl;
		return false;
	}

	int errors = 0;
	for (unsigned int i = 0; i < left.size(); ++i)
	{
		if (!(left[i] == right[i]))
		{
			std::cout << "triangle " << i << " is different" << std::endl;
			++errors;
		}
	}
	return errors == 0;
}

int main()
{
	Mesh::VertexList ascii_vertices;
	Mesh::TriangleIndicesList ascii_triangles;
	readply(L"../test/data/test.ply", ascii_vertices, ascii_triangles);

	Mesh::VertexList bin_vertices;
	Mesh::TriangleIndicesList bin_triangles;
	readply(L"../test/data/test_bin.ply", bin_vertices, bin_triangles);

	compare_vertices(ascii_vertices, bin_vertices);
	compare_triangles(ascii_triangles, bin_triangles);

	libply::File refFile(L"../test/data/test.ply");
	writeply(L"../test/results/write_ascii.ply", refFile.definitions(), ascii_vertices, ascii_triangles, libply::File::Format::ASCII);
	writeply(L"../test/results/write_bin.ply", refFile.definitions(), ascii_vertices, ascii_triangles, libply::File::Format::BINARY_LITTLE_ENDIAN);
}
