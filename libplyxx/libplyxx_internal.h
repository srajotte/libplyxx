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

	typedef std::map<std::string, IElementInserter*> InserterMap;

	/// Type conversion functions.

	inline void convert_UCHAR(const textio::SubString& token, IScalarProperty& property)
	{
		property = textio::stou<unsigned char>(token);
	}

	inline void convert_INT(const textio::SubString& token, IScalarProperty& property)
	{
		property = textio::stoi<int>(token);
	}

	inline void convert_FLOAT(const textio::SubString& token, IScalarProperty& property)
	{
		property = textio::stor<float>(token);
	}

	inline void convert_DOUBLE(const textio::SubString& token, IScalarProperty& property)
	{
		property = textio::stor<double>(token);
	}

	typedef void(*ConversionFunction)(const textio::SubString&, IScalarProperty&);
	typedef std::unordered_map<Type, ConversionFunction> ConversionFunctionMap;

	const ConversionFunctionMap CONVERSION_MAP =
	{
		{ Type::UCHAR , convert_UCHAR },
		{ Type::INT, convert_INT },
		{ Type::FLOAT, convert_FLOAT },
		{ Type::DOUBLE, convert_DOUBLE }
	};

	/// Type casting functions.

	inline void cast_UCHAR(char* buffer, IScalarProperty& property)
	{
		property = *reinterpret_cast<unsigned char*>(buffer);
	}

	inline void cast_INT(char* buffer, IScalarProperty& property)
	{
		property = *reinterpret_cast<int*>(buffer);
	}

	inline void cast_FLOAT(char* buffer, IScalarProperty& property)
	{
		property = *reinterpret_cast<float*>(buffer);
	}

	inline void cast_DOUBLE(char* buffer, IScalarProperty& property)
	{
		property = *reinterpret_cast<double*>(buffer);
	}

	typedef void(*CastFunction)(char* buffer, IScalarProperty&);
	typedef std::unordered_map<Type, CastFunction> CastFunctionMap;

	const CastFunctionMap CAST_MAP =
	{
		{ Type::UCHAR , cast_UCHAR },
		{ Type::INT, cast_INT },
		{ Type::FLOAT, cast_FLOAT },
		{ Type::DOUBLE, cast_DOUBLE }
	};

	struct PropertyDefinition
	{
		PropertyDefinition(const std::string& name, Type type, bool isList, Type listLengthType = Type::UCHAR)
			: name(name), type(type), isList(isList), listLengthType(listLengthType),
			conversionFunction(CONVERSION_MAP.at(type)),
			castFunction(CAST_MAP.at(type))
		{};

		Property getProperty() const;

		std::string name;
		Type type;
		bool isList;
		Type listLengthType;
		ConversionFunction conversionFunction;
		CastFunction castFunction;
	};

	struct ElementDefinition
	{
		ElementDefinition() : ElementDefinition("", 0, 0) {};
		ElementDefinition(const std::string& name, ElementSize size, std::size_t startLine)
			: name(name), size(size), startLine(startLine) {};

		Element getElement() const;

		std::string name;
		ElementSize size;
		std::vector<PropertyDefinition> properties;
		std::size_t startLine;
	};

	class FileParser
	{
	public:
		explicit FileParser(const std::wstring& filename);
		FileParser(const FileParser& other) = delete;
		~FileParser();
		
		std::vector<Element> definitions() const;
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