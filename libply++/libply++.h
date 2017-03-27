#pragma once

#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <cassert>

#include "textio.h"

namespace libply
{
	typedef std::size_t ElementSize;

	enum class Type
	{
		//CHAR,
		UCHAR,
		//SHORT,
		//USHORT,
		INT,
		//UINT,
		FLOAT,
		DOUBLE,
		//INT8,
		//UINT8,
		//INT16,
		//UINT16,
		//INT32,
		//UINT32,
		//FLOAT32,
		//FLOAT64
	};

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

	class IProperty
	{
	public:
		virtual IProperty& operator=(unsigned int value) = 0;
		virtual IProperty& operator=(int value) = 0;
		virtual IProperty& operator=(float value) = 0;
		virtual IProperty& operator=(double value) = 0;
	};

	template<typename InternalType>
	class ScalarProperty: public IProperty
	{
	public :
		virtual ScalarProperty& operator=(unsigned int value)
			{ m_scalar = static_cast<InternalType>(value); return *this; };
		virtual ScalarProperty& operator=(int value)
			{ m_scalar = static_cast<InternalType>(value); return *this; };
		virtual ScalarProperty& operator=(float value)
			{ m_scalar = static_cast<InternalType>(value); return *this; };
		virtual ScalarProperty& operator=(double value)
			{ m_scalar = static_cast<InternalType>(value); return *this; };

		InternalType value() const { return m_scalar; };
	private :
		InternalType m_scalar;
	};

	typedef std::unordered_map<std::size_t, IProperty*> PropertyMap;

	class IElementInserter
	{
	public:
		virtual PropertyMap properties() = 0;
		virtual void insert() = 0;
	};

	///////////////////////////////////////////////////////////////////////////

	typedef std::map<std::string, IElementInserter*> InserterMap;

	inline void convert_UCHAR(const textio::SubString& token, IProperty& property)
	{
		property = textio::stou<unsigned char>(token);
	}

	inline void convert_INT(const textio::SubString& token, IProperty& property)
	{
		property = textio::stoi<int>(token);
	}

	inline void convert_FLOAT(const textio::SubString& token, IProperty& property)
	{
		property = textio::stor<float>(token);
	}

	inline void convert_DOUBLE(const textio::SubString& token, IProperty& property)
	{
		property = textio::stor<double>(token);
	}

	typedef void(*ConversionFunction)(const textio::SubString&, IProperty&);
	typedef std::unordered_map<Type, ConversionFunction> ConversionFunctionMap;

	const ConversionFunctionMap CONVERSION_MAP =
	{
		{ Type::UCHAR , convert_UCHAR },
		{ Type::INT, convert_INT },
		{ Type::FLOAT, convert_FLOAT },
		{ Type::DOUBLE, convert_DOUBLE }
	};

	inline void cast_UCHAR(char* buffer, IProperty& property)
	{
		property = *reinterpret_cast<unsigned char*>(buffer);
	}

	inline void cast_INT(char* buffer, IProperty& property)
	{
		property = *reinterpret_cast<int*>(buffer);
	}

	inline void cast_FLOAT(char* buffer, IProperty& property)
	{
		property = *reinterpret_cast<float*>(buffer);
	}

	inline void cast_DOUBLE(char* buffer, IProperty& property)
	{
		property = *reinterpret_cast<double*>(buffer);
	}

	typedef void(*CastFunction)(char* buffer, IProperty&);
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
		PropertyDefinition(const std::string& name, Type type, bool isList)
			: name(name), type(type), isList(isList), 
			  conversionFunction(CONVERSION_MAP.at(type)), 
			  castFunction(CAST_MAP.at(type))
			  {};

		std::string name;
		Type type;
		bool isList;
		ConversionFunction conversionFunction;
		CastFunction castFunction;
	};

	struct ElementDefinition
	{
		ElementDefinition() : ElementDefinition("", 0, 0) {};
		ElementDefinition(const std::string& name, ElementSize size, std::size_t startLine)
			: name(name), size(size), startLine(startLine) {};

		std::string name;
		ElementSize size;
		std::vector<PropertyDefinition> properties;
		std::size_t startLine;
	};
	
	class File
	{
	public:
		File(const std::wstring& filename);

		const std::vector<ElementDefinition>& definitions() const { return m_elements;} ;
		const void readElements(const InserterMap& im);

	private:
		void readHeader();
		void parseLine(const textio::SubString& substr, const ElementDefinition& elementDefinition, const PropertyMap& am);
		void readBinaryElement(std::ifstream& fs, const ElementDefinition& elementDefinition, const PropertyMap& am);

	private:
		enum class Format
		{
			ASCII,
			BINARY_LITTLE_ENDIAN,
			BINARY_BIG_ENDIAN
		};
	private:
		std::wstring m_filename;
		File::Format m_format;
		std::streamsize m_dataOffset;
		textio::LineReader m_lineReader;
		textio::Tokenizer m_lineTokenizer;
		textio::Tokenizer::TokenList m_tokens;
		std::vector<ElementDefinition> m_elements;
	};
}