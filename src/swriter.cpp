/* swwriter.cpp - Copyright (C) 2024 a dinosaur (zlib, see COPYING.txt) */

#include "swriter.hpp"
#include <type_traits>
#include <limits>
#include <assert.h>

#define GNU_STYLE  0
#define MASM_STYLE 1

#define HEX_STYLE GNU_STYLE


static inline constexpr char HexU(uint8_t h) { return "0123456789ABCDEF"[h >> 4]; }
static inline constexpr char HexL(uint8_t l) { return "0123456789ABCDEF"[l & 15]; }

#if HEX_STYLE == GNU_STYLE
template <typename T> static void CHex(std::ostream& s, T x);
template <> void CHex(std::ostream& s, uint8_t x)
{
	if (x >  9) s << "0x";
	if (x > 15) s << HexU(x);
	s << HexL(x);
}
template <> void CHex(std::ostream& s, uint16_t x)
{
	if (x >    9) s << "0x";
	if (x > 4095) s << HexU(static_cast<uint8_t>(x >> 8));
	if (x >  255) s << HexL(static_cast<uint8_t>(x >> 8));
	if (x >   15) s << HexU(static_cast<uint8_t>(x));
	s << HexL(static_cast<uint8_t>(x));
}
template <> void CHex(std::ostream& s, uint32_t x)
{
	if (x >         9) s << "0x";
	if (x > 0xFFFFFFF) s << HexU(static_cast<uint8_t>(x >> 24));
	if (x >  0xFFFFFF) s << HexL(static_cast<uint8_t>(x >> 24));
	if (x >   0xFFFFF) s << HexU(static_cast<uint8_t>(x >> 16));
	if (x >     65535) s << HexL(static_cast<uint8_t>(x >> 16));
	if (x >      4095) s << HexU(static_cast<uint8_t>(x >>  8));
	if (x >       255) s << HexL(static_cast<uint8_t>(x >>  8));
	if (x >        15) s << HexU(static_cast<uint8_t>(x));
	s << HexL(static_cast<uint8_t>(x));
}
#elif HEX_STYLE == MASM_STYLE
template <typename T> static void MHex(std::ostream& s, T x);
template <> void MHex(std::ostream& s, uint8_t x)
{
	if (x > 159) s << "0";
	if (x >  15) s << HexU(x); else if (x > 9) s << "0";
	s << HexL(x);
	if (x >   9) s << "h";
}
template <> void MHex(std::ostream& s, uint16_t x)
{
	if (x > 40959) s << "0";
	if (x >  4095) s << HexU(static_cast<uint8_t>(x >> 8)); else if (x > 2559) s << "0";
	if (x >   255) s << HexL(static_cast<uint8_t>(x >> 8)); else if (x >  159) s << "0";
	if (x >    15) s << HexU(static_cast<uint8_t>(x));      else if (x >    9) s << "0";
	s << HexL(static_cast<uint8_t>(x));
	if (x >     9) s << "h";
}
template <> void MHex(std::ostream& s, uint32_t x)
{
	if (x > 0x9FFFFFFF) s << "0";
	if (x >  0xFFFFFFF) s << HexU(static_cast<uint8_t>(x >> 24)); else if (x > 0x9FFFFFF) s << "0";
	if (x >   0xFFFFFF) s << HexL(static_cast<uint8_t>(x >> 24)); else if (x >  0x9FFFFF) s << "0";
	if (x >    0xFFFFF) s << HexU(static_cast<uint8_t>(x >> 16)); else if (x >    655359) s << "0";
	if (x >      65535) s << HexL(static_cast<uint8_t>(x >> 16)); else if (x >     40959) s << "0";
	if (x >       4095) s << HexU(static_cast<uint8_t>(x >>  8)); else if (x >      2559) s << "0";
	if (x >        255) s << HexL(static_cast<uint8_t>(x >>  8)); else if (x >       159) s << "0";
	if (x >         15) s << HexU(static_cast<uint8_t>(x));       else if (x >         9) s << "0";
	s << HexL(static_cast<uint8_t>(x));
	if (x >          9) s << "h";
}
#else
# error "Unknown hex style"
#endif


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
#if HEX_STYLE == MASM_STYLE
		MHex(s, e);
#elif HEX_STYLE == GNU_STYLE
		CHex(s, e);
#endif

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
