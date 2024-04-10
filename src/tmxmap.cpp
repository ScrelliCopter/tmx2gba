// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#include "tmxmap.hpp"
#include <pugixml.hpp>
#include <base64.h>
#ifdef USE_ZLIB
# include <zlib.h>
#else
# include "gzip.hpp"
#endif
#include <zstd.h>
#include <limits>
#include <cerrno>
#include <optional>
#include <algorithm>


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

[[nodiscard]] static std::optional<std::string> UnBase64(const std::string_view base64)
{
	// Cut leading & trailing whitespace (including newlines)
	auto beg = std::find_if_not(base64.begin(), base64.end(), ::isspace);
	if (beg == std::end(base64)) { return std::nullopt; }
	auto end = std::find_if_not(base64.rbegin(), base64.rend(), ::isspace);
	auto begOff = std::distance(base64.begin(), beg);
	auto endOff = std::distance(end, base64.rend()) - begOff;
	using size_type = std::string::size_type;
	const auto trimmed = base64.substr(static_cast<size_type>(begOff), static_cast<size_type>(endOff));

	// Decode base64 string
	return base64_decode(trimmed);
}

enum class Encoding { XML, BASE64, CSV, INVALID };
enum class Compression { NONE, GZIP, ZLIB, ZSTD, INVALID };

[[nodiscard]] static Encoding EncodingFromStr(const std::string_view str)
{
	if (str.empty())     { return Encoding::XML; }
	if (str == "base64") { return Encoding::BASE64; }
	if (str == "csv")    { return Encoding::CSV; }
	return Encoding::INVALID;
}

[[nodiscard]] static Compression CompressionFromStr(const std::string_view str)
{
	if (str.empty())   { return Compression::NONE; }
	if (str == "gzip") { return Compression::GZIP; }
	if (str == "zlib") { return Compression::ZLIB; }
	if (str == "zstd") { return Compression::ZSTD; }
	return Compression::INVALID;
}

[[nodiscard]] static bool Decompress(Compression compression, std::span<uint32_t> out, const std::string_view decoded)
{
	//FIXME: lmao what is big endian
	const std::span source(reinterpret_cast<const uint8_t*>(decoded.data()), decoded.size());
	std::span destination(reinterpret_cast<uint8_t*>(out.data()), sizeof(uint32_t) * out.size());

	switch (compression)
	{
	case Compression::GZIP:
#ifndef USE_ZLIB
		{
			GZipReader reader;
			if (!reader.OpenMemory(source) || !reader.Read(destination) || !reader.Check())
				return false;
			return true;
		}
#endif
	case Compression::ZLIB:
		{
			// Decompress gzip/zlib data with zlib/zlib data miniz
			z_stream s =
			{
				.next_in  = const_cast<Bytef*>(source.data()),
				.avail_in = static_cast<unsigned int>(source.size()),
				.next_out  = static_cast<Bytef*>(destination.data()),
				.avail_out = static_cast<unsigned int>(destination.size()),
				.zalloc = nullptr, .zfree = nullptr, .opaque = nullptr
			};
#ifdef USE_ZLIB
			const int wbits = (compression == Compression::GZIP) ? MAX_WBITS | 16 : MAX_WBITS;
#else
			const int wbits = MZ_DEFAULT_WINDOW_BITS;
#endif
			if (inflateInit2(&s, wbits) != Z_OK)
				return false;
			int res = inflate(&s, Z_FINISH);
			inflateEnd(&s);
			return res == Z_STREAM_END;
		}
	case Compression::ZSTD:
		{
			auto res = ZSTD_decompress(
				destination.data(), destination.size(),
				source.data(), source.size());
			return !ZSTD_isError(res);
		}
	// Define all labels to shut up linters
	case Compression::NONE:
	case Compression::INVALID:
	//default:
		return false;
	}
}

void TmxMap::ReadTileset(const pugi::xml_node& xNode)
{
	std::string_view name   = xNode.attribute("name").value();
	std::string_view source = xNode.attribute("source").value();

	auto firstGid = UintFromStr<uint32_t>(xNode.attribute("firstgid").value()).value_or(0);
	auto numTiles = UintFromStr<uint32_t>(xNode.attribute("tilecount").value()).value_or(0);
	if (numTiles == 0)
		return; // FIXME: warn about empty tilesets or something

	mTilesets.emplace_back(TmxTileset(name, source, firstGid, numTiles));
}

void TmxMap::ReadLayer(const pugi::xml_node& xNode)
{
	std::string_view name = xNode.attribute("name").value();

	// Read layer size
	int width  = IntFromStr<int>(xNode.attribute("width").value()).value_or(0);
	int height = IntFromStr<int>(xNode.attribute("height").value()).value_or(0);
	if (width <= 0 || height <= 0) { return; }
	const auto numTiles = static_cast<size_t>(width) * static_cast<size_t>(height);

	auto xData = xNode.child("data");
	if (xData.empty() || xData.first_child().empty())
		return;

	// Read data
	std::vector<uint32_t> tileDat;
	auto encoding = EncodingFromStr(xData.attribute("encoding").value());
	if (encoding == Encoding::BASE64)
	{
		// Decode base64 string
		auto decoded = UnBase64(xData.child_value());
		if (!decoded.has_value())
			return;

		auto compression = CompressionFromStr(xData.attribute("compression").value());
		if (compression == Compression::GZIP || compression == Compression::ZLIB || compression == Compression::ZSTD)
		{
			tileDat.resize(numTiles);
			if (!Decompress(compression, tileDat, decoded.value()))
				return;
		}
		else if (compression == Compression::NONE)
		{
			tileDat.reserve(numTiles);
			const auto end = decoded.value().end();
			for (auto it = decoded.value().begin(); it < end - 3;)
			{
				uint32_t tile = static_cast<uint32_t>(static_cast<uint8_t>(*it++));
				tile |= static_cast<uint32_t>(static_cast<uint8_t>(*it++)) << 8u;
				tile |= static_cast<uint32_t>(static_cast<uint8_t>(*it++)) << 16u;
				tile |= static_cast<uint32_t>(static_cast<uint8_t>(*it++)) << 24u;
				tileDat.emplace_back(tile);
			}
		}
		else { return; }
	}
	else if (encoding == Encoding::XML)
	{
		tileDat.reserve(numTiles);
		std::ranges::transform(xData.children("tile"), std::back_inserter(tileDat), [](auto it)
		-> uint32_t { return UintFromStr<uint32_t>(it.attribute("gid").value()).value_or(0); });
	}
	else if (encoding == Encoding::CSV)
	{
		tileDat.reserve(numTiles);
		const std::string_view csv(xData.child_value());

		std::string::size_type pos = 0;
		while (true)
		{
			// TODO: check if this has a problem on other locales?
			auto gid = UintFromStr<uint32_t>(csv.substr(pos).data());
			if (gid.has_value())
				tileDat.emplace_back(gid.value());

			if ((pos = csv.find(',', pos)) == std::string::npos)
				break;
			++pos;
		}
	}
	else { return; }

	mLayers.emplace_back(TmxLayer(width, height, name, std::move(tileDat)));
}

void TmxMap::ReadObjects(const pugi::xml_node& xNode)
{
	for (const auto it : xNode.children("object"))
	{
		std::string_view name = it.attribute("name").value();

		// Read position
		auto x = FloatFromStr<float>(it.attribute("x").value()).value_or(0.0f);
		auto y = FloatFromStr<float>(it.attribute("y").value()).value_or(0.0f);

		mObjects.emplace_back(TmxObject(name, x, y));
	}
}

bool TmxMap::Load(const std::string& inPath)
{
	// Parse document
	pugi::xml_document xDoc;
	auto res = xDoc.load_file(inPath.c_str());
	if (res.status != pugi::xml_parse_status::status_ok)
		return false;

	// Get map node
	auto xMap = xDoc.child("map");
	if (xMap.empty())
		return false;

	// Read map attribs
	mWidth  = IntFromStr<int>(xMap.attribute("width").value()).value_or(0);
	mHeight = IntFromStr<int>(xMap.attribute("height").value()).value_or(0);

	// Read nodes
	for (auto it : xMap.children())
	{
		std::string_view name(it.name());
		if      (!name.compare("layer"))       { ReadLayer(it); }
		else if (!name.compare("tileset"))     { ReadTileset(it); }
		else if (!name.compare("objectgroup")) { ReadObjects(it); }
	}

	return true;
}
