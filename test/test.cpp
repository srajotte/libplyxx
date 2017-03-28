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

class VertexInserter : public libply::IElementInserter
{
public:
	VertexInserter(Mesh::VertexList& vertices);

	// Implements IElementInserter.
	virtual libply::PropertyMap properties() override;
	virtual void insert() override;

private:	
	libply::ScalarProperty<double> x;
	libply::ScalarProperty<double> y;
	libply::ScalarProperty<double> z;
	Mesh::VertexList& m_vertices;
};

class TriangleInserter : public libply::IElementInserter
{
public:
	TriangleInserter(Mesh::TriangleIndicesList& vertices);

	// Implements IElementInserter.
	virtual libply::PropertyMap properties() override;
	virtual void insert() override;

private:
	libply::ScalarProperty<unsigned int> v0;
	libply::ScalarProperty<unsigned int> v1;
	libply::ScalarProperty<unsigned int> v2;
	Mesh::TriangleIndicesList& m_triangles;
};

VertexInserter::VertexInserter(Mesh::VertexList& vertices) 
	: m_vertices(vertices)
{

}

libply::PropertyMap VertexInserter::properties()
{
	libply::PropertyMap pm{{ 0, &x },{ 1, &y },{ 2, &z }};
	return pm;
}

void VertexInserter::insert()
{
	m_vertices.emplace_back(x.value(), y.value(), z.value());
}

TriangleInserter::TriangleInserter(Mesh::TriangleIndicesList& triangles)
	: m_triangles(triangles)
{

}

libply::PropertyMap TriangleInserter::properties()
{
	libply::PropertyMap pm{{0, &v0},{1, &v1},{2, &v2}};
	return pm;
}

void TriangleInserter::insert()
{
	// Must construct and move temporary object, because std::array doesn't have an initializer list constructor.
	m_triangles.emplace_back(std::move(Mesh::TriangleIndices{ v0.value(), v1.value(), v2.value() }));
}

void readply(std::wstring filename, Mesh::VertexList& vertices, Mesh::TriangleIndicesList& triangles)
{
	libply::File file(filename);

	const auto& definitions = file.definitions();

	const auto vertexDefinition = definitions.at(0);
	const size_t vertexCount = vertexDefinition.size;
	vertices.reserve(vertexCount);
	VertexInserter vertexInserter(vertices);

	const auto triangleDefinition = definitions.at(1);
	const size_t triangleCount = triangleDefinition.size;
	triangles.reserve(triangleCount);
	TriangleInserter triangleInserter(triangles);

	file.setElementInserter("vertex", &vertexInserter);
	file.setElementInserter("face", &triangleInserter);
	file.read();
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
}
