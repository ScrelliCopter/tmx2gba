// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2024 a dinosaur

#ifndef STRTOOLS_HPP
#define STRTOOLS_HPP

#include <string>
#include <string_view>

// Cut leading & trailing whitespace (including newlines)
[[nodiscard]] const std::string_view TrimWhitespace(const std::string_view str);

// Convert string to valid C identifier
[[nodiscard]] std::string SanitiseLabel(const std::string_view ident);


#include <ostream>

// Template functions for converting unsigned ints to C/GNU style hex

static inline constexpr char CHexU(uint8_t h) { return "0123456789ABCDEF"[h >> 4]; }
static inline constexpr char CHexL(uint8_t l) { return "0123456789ABCDEF"[l & 15]; }

template <typename T> static void CHex(std::ostream& s, T x);
template <> void CHex(std::ostream& s, uint8_t x)
{
	if (x >  9) s << "0x";
	if (x > 15) s << CHexU(x);
	s << CHexL(x);
}
template <> void CHex(std::ostream& s, uint16_t x)
{
	if (x >    9) s << "0x";
	if (x > 4095) s << CHexU(static_cast<uint8_t>(x >> 8));
	if (x >  255) s << CHexL(static_cast<uint8_t>(x >> 8));
	if (x >   15) s << CHexU(static_cast<uint8_t>(x));
	s << CHexL(static_cast<uint8_t>(x));
}
template <> void CHex(std::ostream& s, uint32_t x)
{
	if (x >         9) s << "0x";
	if (x > 0xFFFFFFF) s << CHexU(static_cast<uint8_t>(x >> 24));
	if (x >  0xFFFFFF) s << CHexL(static_cast<uint8_t>(x >> 24));
	if (x >   0xFFFFF) s << CHexU(static_cast<uint8_t>(x >> 16));
	if (x >     65535) s << CHexL(static_cast<uint8_t>(x >> 16));
	if (x >      4095) s << CHexU(static_cast<uint8_t>(x >>  8));
	if (x >       255) s << CHexL(static_cast<uint8_t>(x >>  8));
	if (x >        15) s << CHexU(static_cast<uint8_t>(x));
	s << CHexL(static_cast<uint8_t>(x));
}


#include <limits>
#include <cstdlib>
#include <optional>

// Templated string to int/float w/ exception-less error handling

template <typename T>
[[nodiscard]] static std::optional<T> IntFromStr(const char* str, int base = 0) noexcept
{
	using std::numeric_limits;

	errno = 0;
	char* end = nullptr;
	long res = std::strtol(str, &end, base);
	if (errno == ERANGE) { return std::nullopt; }
	if (str == end) { return std::nullopt; }
	if constexpr (sizeof(long) > sizeof(T))
	{
		if (res > numeric_limits<T>::max() || res < numeric_limits<T>::min())
			return std::nullopt;
	}

	return static_cast<T>(res);
}

template <typename T>
[[nodiscard]] static std::optional<T> UintFromStr(const char* str, int base = 0) noexcept
{
	using std::numeric_limits;

	char* end = nullptr;
	errno = 0;
	unsigned long res = std::strtoul(str, &end, base);
	if (errno == ERANGE) { return std::nullopt; }
	if (str == end) { return std::nullopt; }
	if constexpr (numeric_limits<unsigned long>::max() > numeric_limits<T>::max())
	{
		if (res > numeric_limits<T>::max()) { return std::nullopt; }
	}

	return static_cast<T>(res);
}

template <typename T>
[[nodiscard]] static std::optional<T> FloatFromStr(const char* str) noexcept
{
	char* end = nullptr;
	T res;
	errno = 0;
	if constexpr (std::is_same_v<T, float>)
		res = std::strtof(str, &end);
	else
		res = static_cast<T>(std::strtod(str, &end));
	if (errno == ERANGE) { return std::nullopt; }
	if (str == end) { return std::nullopt; }

	return res;
}

#endif//STRTOOLS_HPP
