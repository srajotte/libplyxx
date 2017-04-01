#pragma once

#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <cassert>
#include <memory>

#include "textio.h"

namespace libply
{
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

	class ElementBuffer
	{
	public:
		void appendScalarProperty(const std::string& name, Type type);
		//void appendPropertyList(const std::string& name, Type type, std::size_t size);

	public:
		std::vector<std::string> propertyNames;
		std::vector<std::unique_ptr<IProperty>> properties;
	};

	class IElementInserter
	{
	public:
		virtual PropertyMap properties() = 0;
		virtual void insert() = 0;
	};
	
	typedef std::size_t ElementSize;

	struct Property
	{
		Property(const std::string& name, Type type, bool isList)
			: name(name), type(type), isList(isList) {};

		std::string name;
		Type type;
		bool isList;
	};

	struct Element
	{
		Element(const std::string& name, ElementSize size, const std::vector<Property>& properties)
			: name(name), size(size), properties(properties) {};

		std::string name;
		ElementSize size;
		std::vector<Property> properties;
	};

	class FileParser;

	class File
	{
	public:
		File(const std::wstring& filename);
		~File();

		std::vector<Element> definitions() const;
		void setElementInserter(std::string elementName, IElementInserter* inserter);
		void read();

	public:
		enum class Format
		{
			ASCII,
			BINARY_LITTLE_ENDIAN,
			BINARY_BIG_ENDIAN
		};

	private:
		std::wstring m_filename;
		std::unique_ptr<FileParser> m_parser;
	};
}