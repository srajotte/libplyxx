#include "libply++.h"

#include <fstream>
#include <string>

namespace libply
{
File::File(const std::wstring& filename)
	: m_filename(filename),
	m_lineTokenizer(' '),
	m_lineReader(filename)
{
	readHeader();
}

void addElementDefinition(const textio::Tokenizer::TokenList& tokens, std::vector<ElementDefinition>& elementDefinitions)
{
	assert(std::string(tokens.at(0)) == "element");
	size_t startLine = 0;
	if (!elementDefinitions.empty())
	{
		const auto& previousElement = elementDefinitions.back();
		startLine = previousElement.startLine + previousElement.size;
	}
	ElementSize elementCount = std::stoul(tokens.at(2));
	elementDefinitions.emplace_back(tokens.at(1), elementCount, startLine);
}

void addProperty(const textio::Tokenizer::TokenList& tokens, ElementDefinition& elementDefinition)
{
	auto& properties = elementDefinition.properties;
	if (std::string(tokens.at(1)) == "list")
	{
		properties.emplace_back(tokens.back(), TYPE_MAP.at(tokens.at(3)), true);
	}
	else
	{
		properties.emplace_back(tokens.back(), TYPE_MAP.at(tokens.at(1)), false);
	}
}

void File::readHeader()
{
	// Read PLY magic number.
	std::string line = m_lineReader.getline();
	if (line != "ply")
	{
		throw std::runtime_error("Invalid file format.");
	}

	// Read file format.
	line = m_lineReader.getline();
	if (line != "format ascii 1.0")
	{
		throw std::runtime_error("Unsupported PLY format : " + line);
	}

	// Read mesh elements properties.
	line = m_lineReader.getline();
	textio::Tokenizer spaceTokenizer(' ');
	auto tokens = spaceTokenizer.tokenize(line);
	size_t startLine = 0;
	while (std::string(tokens.at(0)) != "end_header")
	{
		const std::string lineType = tokens.at(0);
		if (lineType == "element")
		{
			addElementDefinition(tokens, m_elements);
		}
		else if (lineType == "property")
		{
			addProperty(tokens, m_elements.back());
		}
		else
		{
			//throw std::runtime_error("Invalid header line.");
		}

		line = m_lineReader.getline();
		tokens = spaceTokenizer.tokenize(line);
	}
}

const void File::readElements(const InserterMap& im)
{
	std::size_t totalLines = 0;
	for (auto& e : m_elements)
	{
		totalLines += e.size;
	}

	std::size_t lineIndex = 0;
	std::size_t elementIndex = 0;
	IElementInserter* elementInserter = im.at(m_elements.at(elementIndex).name);
	PropertyMap properties = elementInserter->properties();
	auto& elementDefinition = m_elements.at(elementIndex);
	const std::size_t maxElementIndex = m_elements.size();

	while (lineIndex < totalLines)
	{
		auto line = m_lineReader.getline();

		const auto nextElementIndex = elementIndex + 1;
		if (nextElementIndex < maxElementIndex && lineIndex >= m_elements[nextElementIndex].startLine)
		{
			elementIndex = nextElementIndex;
			elementInserter = im.at(m_elements.at(elementIndex).name);
			elementDefinition = m_elements.at(elementIndex);
			properties = elementInserter->properties();
		}

		parseLine(line, elementDefinition, properties);
		elementInserter->insert();
		++lineIndex;
	}
}

void File::parseLine(const textio::SubString& line, const ElementDefinition& elementDefinition, const PropertyMap& pm)
{
	m_lineTokenizer.tokenize(line, m_tokens);
	const auto& properties = elementDefinition.properties;

	if (!properties.front().isList)
	{
		for (auto& kv : pm)
		{
			auto i = kv.first;
			properties[i].conversionFunction(m_tokens[i], *kv.second);
		}
	}
	else
	{
		const auto& conversionFunction = properties[0].conversionFunction;
		for (auto& kv : pm)
		{
			auto i = kv.first + 1;
			conversionFunction(m_tokens[i], *kv.second);
		}
	}
}
}