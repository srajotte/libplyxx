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

	class IScalarProperty
	{
	public:
		virtual IScalarProperty& operator=(unsigned int value) = 0;
		virtual IScalarProperty& operator=(int value) = 0;
		virtual IScalarProperty& operator=(float value) = 0;
		virtual IScalarProperty& operator=(double value) = 0;
	};

	template<typename InternalType>
	class ScalarProperty: public IScalarProperty
	{
	public :
		virtual ScalarProperty& operator=(unsigned int value) override
			{ m_value = static_cast<InternalType>(value); return *this; };
		virtual ScalarProperty& operator=(int value) override
			{ m_value = static_cast<InternalType>(value); return *this; };
		virtual ScalarProperty& operator=(float value) override
			{ m_value = static_cast<InternalType>(value); return *this; };
		virtual ScalarProperty& operator=(double value) override
			{ m_value = static_cast<InternalType>(value); return *this; };

	public:
		InternalType value() const { return m_value; };

	private :
		InternalType m_value;
	};

	class IListProperty
	{
	public:
		virtual void reset(size_t size) = 0;
		virtual IScalarProperty& operator[](size_t index) = 0;
	};

	template<typename InternalType>
	class ListProperty : public IListProperty
	{
	public:
		ListProperty() {};
		ListProperty(size_t size) : m_values(size) {};

	public:
		virtual void reset(size_t size) override
			{ m_values.resize(size); };
		virtual ScalarProperty<InternalType>& operator[](size_t index) override
			{ return m_values[index]; };

	private:
		std::vector<ScalarProperty<InternalType>> m_values;
	};

	typedef std::unordered_map<std::size_t, IScalarProperty*> PropertyMap;

	struct ElementDefinition;

	class ElementBuffer
	{
	public:
		ElementBuffer() = default;
		ElementBuffer(const ElementDefinition& definition);

	public:
		void reset(size_t size);
		IScalarProperty& operator[](size_t index);

	private:
		void appendScalarProperty(Type type);
		void appendListProperty(Type type);
		std::unique_ptr<IScalarProperty> getScalarProperty(Type type);

	private:
		bool m_isList;
		Type m_listType;
		std::vector<std::unique_ptr<IScalarProperty>> properties;
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