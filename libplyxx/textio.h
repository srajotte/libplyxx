#pragma once

#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <cassert>
#include <cmath>
#include <cstring>

namespace textio
{
	class SubString
	{
	public:
		typedef std::string::const_iterator const_iterator;

	public:
		SubString() = default;
		SubString(const std::string::const_iterator& begin, const std::string::const_iterator& end)
			: m_begin(begin), m_end(end) {};

		operator std::string() const { return std::string(m_begin, m_end); };

		const_iterator begin() const { return m_begin; };
		const_iterator end() const { return m_end; };

	private:
		const_iterator m_begin;
		const_iterator m_end;
	};

	class Tokenizer
	{
	public:
		typedef std::vector<SubString> TokenList;
	public:
		inline Tokenizer(char delimiter);

		inline TokenList tokenize(const std::string& buffer) const;
		inline TokenList tokenize(const SubString& buffer) const;
		inline void tokenize(const SubString& buffer, TokenList& tokens) const;

	private:
		char m_delimiter;
	};

	class LineReader
	{
	public:
		template<typename PathString>
		inline LineReader(const PathString& filename, bool textMode = false);

		// Read next line from input file.
		// Returned SubString is valid until the next call to getline()
		inline SubString getline();
		inline bool eof() const { return m_eof; };
		inline std::ifstream& filestream() { return m_file; };
		inline std::streamsize position(const std::string::const_iterator& workbuf_iter);

	private:
		//inline void readFileChunk(std::streamoff offset = 0);
		inline std::streamsize readFileChunk(std::size_t overlap);
		inline SubString findLine();

	private:
		typedef std::string WorkBuffer;

	private:
		std::ifstream m_file;

		std::streamsize m_workBufSize;
		std::streamsize m_workBufFileEndPosition;
		WorkBuffer m_workBuf;
		bool m_eof;

		WorkBuffer::const_iterator m_begin;
		WorkBuffer::const_iterator m_end;
	};

	// Convert string to floating point (real) type.
	template<typename T>
	T stor(const SubString& substr)
	{
		auto p = substr.begin();
		auto end = substr.end();
		T real = 0.0;
		bool negative = false;
		if (p != end && *p == '-')
		{
			negative = true;
			++p;
		}
		while (p != end && *p >= '0' && *p <= '9')
		{
			real = (real*static_cast<T>(10.0)) + (*p - '0');
			++p;
		}
		if (p != end && *p == '.')
		{
			T frac = 0.0;
			int n = 0;
			++p;
			while (p != end && *p >= '0' && *p <= '9')
			{
				frac = (frac*static_cast<T>(10.0)) + (*p - '0');
				++p;
				++n;
			}
			real += frac / std::pow(static_cast<T>(10.0), n);
		}
		if (p != end && (*p == 'e' || *p == 'E'))
		{
			++p;
			T sign = 1.0;
			if (p != end && *p == '-')
			{
				sign = -1.0;
				++p;
			}
			T exponent = 0.0;
			while (p != end && *p >= '0' && *p <= '9')
			{
				exponent = (exponent*static_cast<T>(10.0)) + (*p - '0');
				++p;
			}
			real = real * std::pow(static_cast<T>(10.0), sign * exponent);
		}
		if (negative)
		{
			real = -real;
		}
		return real;
	}

	template<typename T>
	T stor(const std::string& str)
	{
		return stor<T>(SubString(str.cbegin(), str.cend()));
	}

	// Convert string to unsigned type.
	template<typename T>
	T stou(const SubString& substr)
	{
		static_assert(std::is_unsigned<T>::value, "Cannot use stou() with signed type.");
		auto p = substr.begin();
		auto end = substr.end();
		T integer = 0;
		assert(*p != '-');
		while (p != end && *p >= '0' && *p <= '9')
		{
			integer = (integer * 10) + (*p - '0');
			++p;
		}
		return integer;
	}

	// Convert string to signed integer type.
	template<typename T>
	T stoi(const SubString& substr)
	{
		auto p = substr.begin();
		auto end = substr.end();
		T integer = 0;
		bool negative = false;
		if (p != end && *p == '-')
		{
			negative = true;
			++p;
		}
		while (p != end && *p >= '0' && *p <= '9')
		{
			integer = (integer * 10) + (*p - '0');
			++p;
		}
		if (negative)
		{
			integer = -integer;
		}
		return integer;
	}

	template<typename T>
	T stou(const std::string& str)
	{
		return stou<T>(SubString(str.cbegin(), str.cend()));
	}

	Tokenizer::Tokenizer(char delimiter)
		: m_delimiter(delimiter)
	{

	}

	Tokenizer::TokenList Tokenizer::tokenize(const SubString& buffer) const
	{
		TokenList tokens;
		tokenize(buffer, tokens);
		return tokens;
	}

	Tokenizer::TokenList Tokenizer::tokenize(const std::string& buffer) const
	{
		return tokenize(SubString(buffer.cbegin(), buffer.cend()));
	}

	inline textio::SubString::const_iterator find(textio::SubString::const_iterator begin, textio::SubString::const_iterator end, char delimiter)
	{
		auto start = begin;
		while (start != end)
		{
			if (*start == delimiter) return start;
			++start;
		}
		return end;
	}

	inline textio::SubString::const_iterator findSIMD(textio::SubString::const_iterator begin, textio::SubString::const_iterator end, char delimiter)
	{
		uint64_t pattern;
		switch (delimiter)
		{
			case '\n': pattern = 0x0a0a0a0a0a0a0a0aULL; break;
			case ' ': pattern = 0x2020202020202020ULL; break;
			default: throw std::runtime_error("Unsupported delimiter.");
		}

		auto start = begin;
		const size_t WORD_WIDTH = 8;
		while (end - start > WORD_WIDTH)
		{
			// Xor data with pattern to find the bit distance. Matching bytes will be 0x00, otherwise >0x00.
			// When subtracting 0x01 to all bytes, only bytes at 0x00 will underflow to 0xff, i.e. bytes that match the pattern.
			// Apply bitwise-and between that last result and 0x80, i.e. b10000000, to keep bytes >0x80. Since the last ASCII character is 0x79, only the matching bytes that underflowed are kept.
			// Must also test with ~data, because subtraction at 0x00 cause borrowing from the adjacent byte, which might have cause an underflow at that byte.
			uint64_t data = *(uint64_t*)&(*start);
			data = data ^ pattern;
			if ((data - 0x0101010101010101ULL) & ~data & 0x8080808080808080)
			{
				// Delimiter found in sequence.
				return textio::find(start, end, delimiter);
			}
			else
			{
				start += WORD_WIDTH;
			}
		}
		// Remaining data in sequence too small to run SIMD search.
		return textio::find(begin, end, delimiter);
	}

	inline void Tokenizer::tokenize(const SubString& buffer, TokenList& tokens) const
	{
		tokens.clear();
		auto begin = buffer.begin();
		const auto end = buffer.end();
		auto eot = begin;
		while (eot != end)
		{
			// Skip all delimiters.
			while (begin != end && *begin == m_delimiter)
			{
				++begin;
			}
			eot = textio::find(begin, end, m_delimiter);

			tokens.emplace_back(begin, eot);
			if (eot != end)
			{
				// Move begin after delimiter.
				begin = eot + 1;
			}
		}
	}

	template<typename PathString>
	LineReader::LineReader(const PathString& filename, bool textMode)
		: m_workBufSize(1 * 1024 * 1024), m_eof(false), m_workBufFileEndPosition(0)
	{
		auto mode = std::fstream::in;
		if (!textMode) { mode |= std::fstream::binary; }
		m_file.open(filename, mode);
		if (!m_file.is_open())
		{
			throw std::runtime_error("Could not open file.");
		}
		m_workBuf.resize(m_workBufSize);
		readFileChunk(0);
	}

	SubString LineReader::getline()
	{
		return findLine();
	}

	std::streamsize LineReader::readFileChunk(std::size_t overlap)
	{
		char* bufferFront = &m_workBuf.front();
		if (overlap != 0)
		{
			size_t offset = m_workBufSize - overlap;
			std::memcpy(bufferFront, bufferFront + offset, overlap);
		}
		m_file.read(bufferFront + overlap, m_workBufSize - overlap);
		m_begin = m_workBuf.cbegin();
		m_end = m_workBuf.cbegin() + overlap + m_file.gcount();
		m_workBufFileEndPosition += m_file.gcount();
		return m_file.gcount();
	}

	SubString LineReader::findLine()
	{
		SubString::const_iterator eol = findSIMD(m_begin, m_end, '\n');
		if (m_begin == m_workBuf.cbegin() && eol == m_end)
		{
			std::runtime_error("Working buffer too small to fit single line.");
		}
		SubString lineSubstring(m_begin, eol);

		// Reached the end of the work buffer (last character not a newline delimiter).
		if (eol == m_end)
		{
			auto count = readFileChunk(m_end - m_begin);
			if (count == 0 && m_file.eof())
			{
				m_eof = true;
				return SubString(m_begin, m_end);
			}
			else
			{
				lineSubstring = findLine();
			}
		}
		// Line complete.
		else
		{
			// Set begin pointer to the first character after the newline delimiter.
			m_begin = eol + 1;
		}
		return lineSubstring;
	}

	std::streamsize LineReader::position(const std::string::const_iterator& workbuf_iter)
	{
		return m_workBufFileEndPosition - (m_end - workbuf_iter);
	}
}
