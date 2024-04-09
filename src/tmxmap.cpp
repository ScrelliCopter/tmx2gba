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
#include <limits>
#include <cerrno>
#include <optional>


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


bool TmxMap::Decode(std::span<uint32_t> out, const std::string_view base64)
{
	// Cut leading & trailing whitespace (including newlines)
	auto beg = std::find_if_not(base64.begin(), base64.end(), ::isspace);
	if (beg == std::end(base64))
		return false;
	auto end = std::find_if_not(base64.rbegin(), base64.rend(), ::isspace);
	std::size_t begOff = std::distance(base64.begin(), beg);
	std::size_t endOff = std::distance(end, base64.rend()) - begOff;
	const auto trimmed = base64.substr(begOff, endOff);

	// Decode base64 string
	std::string decoded = base64_decode(trimmed);

	// Decompress compressed data
	auto dstSize = static_cast<uLongf>(sizeof(uint32_t) * out.size());
	int res = uncompress(
		reinterpret_cast<unsigned char*>(out.data()),
		&dstSize,
		reinterpret_cast<const unsigned char*>(decoded.data()),
		static_cast<uLong>(decoded.size()));

	return res >= 0;
}

void TmxMap::ReadTileset(const pugi::xml_node& xNode)
{
	std::string_view name   = xNode.attribute("name").value();
	std::string_view source = xNode.attribute("source").value();

	auto firstGid = UintFromStr<uint32_t>(xNode.attribute("firstgid").value()).value_or(0);
	auto lastGid  = UintFromStr<uint32_t>(xNode.attribute("lastgid").value()).value_or(0);

	mTilesets.emplace_back(TmxTileset(name, source, firstGid, lastGid));
}

void TmxMap::ReadLayer(const pugi::xml_node& xNode)
{
	std::string_view name = xNode.attribute("name").value();

	// Read layer size
	int width  = IntFromStr<int>(xNode.attribute("width").value()).value_or(0);
	int height = IntFromStr<int>(xNode.attribute("height").value()).value_or(0);
	if (width <= 0 || height <= 0)
		return;

	// Read tile data
	auto xData = xNode.child("data");
	if (xData.empty() || xData.first_child().empty())
		return;
	// TODO: don't assume base64
	std::vector<uint32_t> tileDat(width * height);
	if (!Decode(tileDat, xData.child_value()))
		return;

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
	//if (xMap == nullptr)
	//	return false;

	// Read map attribs
	mWidth  = IntFromStr<int>(xMap.attribute("width").value()).value_or(0);
	mHeight = IntFromStr<int>(xMap.attribute("height").value()).value_or(0);

	// Read nodes
	//for (auto it = xMap.begin(); it != xMap.end(); ++it)
	for (auto it : xMap.children())
	{
		std::string_view name(it.name());
		if      (!name.compare("layer"))       { ReadLayer(it); }
		else if (!name.compare("tileset"))     { ReadTileset(it); }
		else if (!name.compare("objectgroup")) { ReadObjects(it); }
	}

	return true;
}
