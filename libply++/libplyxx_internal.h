#pragma once

#include "libply++.h"

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
}