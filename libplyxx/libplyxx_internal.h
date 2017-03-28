#pragma once

#include "libplyxx.h"

namespace libply
{
	typedef std::unordered_map<std::string, Type> TypeMap;
	const TypeMap TYPE_MAP =
	{
		{ "uchar", Type::UCHAR },
		{ "int", Type::INT },
		{ "float", Type::FLOAT },
		{ "double", Type::DOUBLE },
	};

	typedef std::unordered_map<Type, unsigned int> TypeSizeMap;
	const TypeSizeMap TYPE_SIZE_MAP =
	{
		{ Type::UCHAR, 1 },
		{ Type::INT, 4 },
		{ Type::FLOAT, 4 },
		{ Type::DOUBLE, 8 },
	};

	class FileParser
	{
	public:
		FileParser(const std::wstring& filename);
		
		const std::vector<ElementDefinition>& definitions() const { return m_elements; };
		void setElementInserter(std::string elementName, IElementInserter* inserter);
		void read();

	private:
		void readHeader();
		void parseLine(const textio::SubString& substr, const ElementDefinition& elementDefinition, const PropertyMap& am);
		void readBinaryElement(std::ifstream& fs, const ElementDefinition& elementDefinition, const PropertyMap& am);

	private:
		std::wstring m_filename;
		File::Format m_format;
		std::streamsize m_dataOffset;
		textio::LineReader m_lineReader;
		textio::Tokenizer m_lineTokenizer;
		textio::Tokenizer::TokenList m_tokens;
		std::vector<ElementDefinition> m_elements;
		InserterMap m_inserterMap;
	};
}