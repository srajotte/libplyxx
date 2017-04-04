#pragma once

#include "libplyxx.h"
#include <sstream>

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

	inline std::stringstream& write_cast_UCHAR(IScalarProperty& property, std::stringstream& ss)
	{
		ss << unsigned int(property);
		return ss;
	}

	inline std::stringstream& write_cast_INT(IScalarProperty& property, std::stringstream& ss)
	{
		ss << int(property);
		return ss;
	}

	inline std::stringstream& write_cast_FLOAT(IScalarProperty& property, std::stringstream& ss)
	{
		ss << float(property);
		return ss;
	}

	inline std::stringstream& write_cast_DOUBLE(IScalarProperty& property, std::stringstream& ss)
	{
		ss << double(property);
		return ss;
	}

	typedef std::stringstream&(*WriteCastFunction)(IScalarProperty&, std::stringstream&);
	typedef std::unordered_map<Type, WriteCastFunction> WriteCastFunctionMap;

	const WriteCastFunctionMap WRITE_CAST_MAP =
	{
		{ Type::UCHAR , write_cast_UCHAR },
		{ Type::INT, write_cast_INT },
		{ Type::FLOAT, write_cast_FLOAT },
		{ Type::DOUBLE, write_cast_DOUBLE }
	};

	struct PropertyDefinition
	{
		PropertyDefinition(const std::string& name, Type type, bool isList, Type listLengthType = Type::UCHAR)
			: name(name), type(type), isList(isList), listLengthType(listLengthType),
			conversionFunction(CONVERSION_MAP.at(type)),
			castFunction(CAST_MAP.at(type)),
			writeCastFunction(WRITE_CAST_MAP.at(type))
		{};
		PropertyDefinition(const Property& p)
			: PropertyDefinition(p.name, p.type, p.isList)
		{};

		Property getProperty() const;

		std::string name;
		Type type;
		bool isList;
		Type listLengthType;
		ConversionFunction conversionFunction;
		CastFunction castFunction;
		WriteCastFunction writeCastFunction;
	};

	struct ElementDefinition
	{
		ElementDefinition() : ElementDefinition("", 0, 0) {};
		ElementDefinition(const std::string& name, ElementSize size, std::size_t startLine)
			: name(name), size(size), startLine(startLine) {};
		ElementDefinition(const Element& e)
			: name(e.name), size(e.size)
		{
			for (const auto& p : e.properties)
			{
				properties.emplace_back(p);
			}
		};

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
		//void setElementInserter(std::string elementName, IElementInserter* inserter);
		void setElementReadCallback(std::string elementName, ElementReadCallback& readCallback);
		void read();

	private:
		void readHeader();
		void parseLine(const textio::SubString& substr, const ElementDefinition& elementDefinition, ElementBuffer& buffer);
		void readBinaryElement(std::ifstream& fs, const ElementDefinition& elementDefinition, ElementBuffer& buffer);

	private:
		typedef std::map<std::string, ElementReadCallback> CallbackMap;

	private:
		std::wstring m_filename;
		File::Format m_format;
		std::streamsize m_dataOffset;
		textio::LineReader m_lineReader;
		textio::Tokenizer m_lineTokenizer;
		textio::Tokenizer::TokenList m_tokens;
		std::vector<ElementDefinition> m_elements;
		CallbackMap m_readCallbackMap;
	};

	std::string formatString(File::Format format);
	std::string typeString(Type type);
}