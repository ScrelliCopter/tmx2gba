// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2024 a dinosaur

#include "swriter.hpp"
#include "strtools.hpp"
#include <type_traits>
#include <assert.h>


template <typename T> static constexpr const std::string_view DataType();
template <> constexpr const std::string_view DataType<uint8_t>()  { return ".byte"; }
template <> constexpr const std::string_view DataType<uint16_t>() { return ".hword"; }
template <> constexpr const std::string_view DataType<uint32_t>() { return ".word"; }

template <typename I>
static void WriteArrayDetail(std::ostream& s, const I beg, const I end, int perCol)
{
	typedef typename std::iterator_traits<I>::value_type Element;

	int col = 0;
	for (auto it = beg;;)
	{
		if (col == 0)
			s << "\t" << DataType<Element>() << " ";

		const Element e = *it;
		CHex(s, e);

		if (++it == end)
			break;

		if (++col < perCol)
		{
			s << ",";
		}
		else
		{
			s << std::endl;
			col = 0;
		}
	}
	s << std::endl;
}


void SWriter::WriteSymbol(const std::string_view suffix)
{
	if (writes++ != 0)
		stream << std::endl;
	stream << "\t.section .rodata" << std::endl;
	stream << "\t.align 2" << std::endl;
	stream << "\t.global " << mName << suffix << std::endl;
	stream << "\t.hidden " << mName << suffix << std::endl;
	stream << mName << suffix << ":" << std::endl;
}

void SWriter::WriteArray(const std::string_view suffix, std::span<uint8_t> data, int numCols)
{
	assert(data.size());
	WriteSymbol(suffix);
	WriteArrayDetail(stream, data.begin(), data.end(), numCols);
}

void SWriter::WriteArray(const std::string_view suffix, std::span<uint16_t> data, int numCols)
{
	assert(data.size());
	WriteSymbol(suffix);
	WriteArrayDetail(stream, data.begin(), data.end(), numCols);
}

void SWriter::WriteArray(const std::string_view suffix, std::span<uint32_t> data, int numCols)
{
	assert(data.size());
	WriteSymbol(suffix);
	WriteArrayDetail(stream, data.begin(), data.end(), numCols);
}


bool SWriter::Open(const std::filesystem::path& path, const std::string_view name)
{
	mName = name;
	stream.open(path);
	return stream.is_open();
}
