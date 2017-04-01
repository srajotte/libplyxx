#include "libplyxx_internal.h"

#include <fstream>
#include <string>

namespace libply
{
File::File(const std::wstring& filename)
	: m_filename(filename),
	m_parser(std::make_unique<FileParser>(filename))
{
}

File::~File() = default;

std::vector<Element> File::definitions() const 
{ 
	return m_parser->definitions(); 
}

void File::setElementInserter(std::string elementName, IElementInserter* inserter) 
{
	m_parser->setElementInserter(elementName, inserter); 
}

void File::read()
{ 
	m_parser->read(); 
};

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
		properties.emplace_back(tokens.back(), TYPE_MAP.at(tokens.at(3)), true, TYPE_MAP.at(tokens.at(2)));
	}
	else
	{
		properties.emplace_back(tokens.back(), TYPE_MAP.at(tokens.at(1)), false);
	}
}

Property PropertyDefinition::getProperty() const
{
	return Property(name, type, isList);
}

Element ElementDefinition::getElement() const
{
	std::vector<Property> properties;
	for (const auto& p : this->properties)
	{
		properties.emplace_back(p.getProperty());
	}
	return Element(name, size, properties);
}

FileParser::FileParser(const std::wstring& filename)
	: m_filename(filename),
	m_lineTokenizer(' '),
	m_lineReader(filename)
{
	readHeader();
}

FileParser::~FileParser() = default;

std::vector<Element> FileParser::definitions() const
{
	std::vector<Element> elements;
	for (const auto& e : m_elements)
	{
		elements.emplace_back(e.getElement());
	}
	return elements;
}

void FileParser::readHeader()
{
	// Read PLY magic number.
	std::string line = m_lineReader.getline();
	if (line != "ply")
	{
		throw std::runtime_error("Invalid file format.");
	}

	// Read file format.
	line = m_lineReader.getline();
	if (line == "format ascii 1.0")
	{
		m_format = File::Format::ASCII;
	}
	else if (line == "format binary_little_endian 1.0")
	{
		m_format = File::Format::BINARY_LITTLE_ENDIAN;
	}
	else if (line == "format binary_big_endian 1.0")
	{
		m_format = File::Format::BINARY_BIG_ENDIAN;
	}
	else
	{
		throw std::runtime_error("Unsupported PLY format : " + line);
	}

	// Read mesh elements properties.
	textio::SubString line_substring;
	line_substring = m_lineReader.getline();
	line = line_substring;
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

		line_substring = m_lineReader.getline();
		line = line_substring;
		tokens = spaceTokenizer.tokenize(line);
	}
	
	m_dataOffset = m_lineReader.position(line_substring.end()) + 1;
}

void FileParser::setElementInserter(std::string elementName, IElementInserter* inserter)
{
	m_inserterMap[elementName] = inserter;
}

void FileParser::read()
{
	std::size_t totalLines = 0;
	for (auto& e : m_elements)
	{
		totalLines += e.size;
	}

	std::size_t lineIndex = 0;
	std::size_t elementIndex = 0;
	IElementInserter* elementInserter = m_inserterMap.at(m_elements.at(elementIndex).name);
	PropertyMap properties = elementInserter->properties();
	auto& elementDefinition = m_elements.at(elementIndex);
	const std::size_t maxElementIndex = m_elements.size();
	
	std::ifstream& filestream = m_lineReader.filestream();

	if (m_format == File::Format::BINARY_BIG_ENDIAN || m_format == File::Format::BINARY_LITTLE_ENDIAN)
	{
		filestream.clear();
		filestream.seekg(m_dataOffset);
	}

	while (lineIndex < totalLines)
	{
		const auto nextElementIndex = elementIndex + 1;
		if (nextElementIndex < maxElementIndex && lineIndex >= m_elements[nextElementIndex].startLine)
		{
			elementIndex = nextElementIndex;
			elementInserter = m_inserterMap.at(m_elements.at(elementIndex).name);
			elementDefinition = m_elements.at(elementIndex);
			properties = elementInserter->properties();
		}

		if (m_format == File::Format::ASCII)
		{
			auto line = m_lineReader.getline();
			parseLine(line, elementDefinition, properties);
		}
		else {
			readBinaryElement(filestream, elementDefinition, properties);
		}
		
		elementInserter->insert();
		++lineIndex;
	}
}

void FileParser::parseLine(const textio::SubString& line, const ElementDefinition& elementDefinition, const PropertyMap& pm)
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

void FileParser::readBinaryElement(std::ifstream& fs, const ElementDefinition& elementDefinition, const PropertyMap& pm)
{
	const auto& properties = elementDefinition.properties;
	const unsigned int MAX_PROPERTY_SIZE = 8;
	char buffer[MAX_PROPERTY_SIZE];

	if (!properties.front().isList)
	{
		for (auto& kv : pm)
		{
			auto i = kv.first;
			const auto size = TYPE_SIZE_MAP.at(properties[i].type);
			fs.read(buffer, size);
			properties[i].castFunction(buffer, *kv.second);
		}
	}
	else
	{
		const auto lengthType = properties[0].listLengthType;
		const auto lengthTypeSize = TYPE_SIZE_MAP.at(lengthType);
		fs.read(buffer, lengthTypeSize);

		const auto& castFunction = properties[0].castFunction;
		const auto size = TYPE_SIZE_MAP.at(properties[0].type);
		for (auto& kv : pm)
		{
			fs.read(buffer, size);
			castFunction(buffer, *kv.second);
		}
	}
}

ElementBuffer::ElementBuffer(const ElementDefinition& definition)
{
	auto ppties = definition.properties;
	for (auto& p : ppties)
	{
		if (p.isList)
		{
			appendListProperty(p.name, p.type);
		}
		else
		{
			appendScalarProperty(p.name, p.type);
		}
	}

}

void ElementBuffer::appendScalarProperty(const std::string& name, Type type)
{
	appendListProperty(name, type);
}

void ElementBuffer::appendListProperty(const std::string& name, Type type)
{
	std::unique_ptr<IListProperty> prop(nullptr);
	switch (type)
	{
	case Type::UCHAR: prop = std::make_unique<ListProperty<char>>(1);  break;
	case Type::INT: prop = std::make_unique<ListProperty<int>>(1); break;
	case Type::FLOAT: prop = std::make_unique<ListProperty<float>>(1); break;
	case Type::DOUBLE: prop = std::make_unique<ListProperty<double>>(1); break;
	}
	properties.push_back(std::move(prop));
	propertyNames.push_back(name);
}

}