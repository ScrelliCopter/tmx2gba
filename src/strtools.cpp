// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#include "strtools.hpp"
#include <cctype>
#include <algorithm>


const std::string_view TrimWhitespace(const std::string_view str)
{
	auto beg = std::find_if_not(str.begin(), str.end(), ::isspace);
	if (beg == std::end(str))
	{
		return {};
	}
	auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace);
	auto begOff = std::distance(str.begin(), beg);
	auto endOff = std::distance(end, str.rend()) - begOff;
	using size_type = std::string::size_type;
	return str.substr(static_cast<size_type>(begOff), static_cast<size_type>(endOff));
}

std::string SanitiseLabel(const std::string_view ident)
{
	std::string out;
	out.reserve(ident.length());

	int last = '_';
	for (int i : ident)
	{
		if (out.empty() && std::isdigit(i)) { continue; }
		if (!std::isalnum(i))               { i = '_'; }
		if (i != '_' || last != '_')        { out.push_back(i); }
		last = i;
	}
	return out;
}
