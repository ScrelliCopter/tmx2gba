// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#include "tmxmap.hpp"
#include "base64.h"
#ifdef USE_ZLIB
# include <zlib.h>
#else
# include "gzip.hpp"
#endif
#include <sstream>
#include <fstream>


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

void TmxMap::ReadTileset(rapidxml::xml_node<>* aXNode)
{
	std::string_view name, source;
	uint32_t firstGid = 0, lastGid = 0;

	// Read name
	auto xAttrib = aXNode->first_attribute("name");
	if (xAttrib != nullptr)
		name = xAttrib->value();

	// Read source
	xAttrib = aXNode->first_attribute("source");
	if (xAttrib != nullptr)
		source = xAttrib->value();

	// Read first global ID
	xAttrib = aXNode->first_attribute("firstgid");
	if (xAttrib != nullptr)
		firstGid = static_cast<uint32_t>(std::stoul(xAttrib->value()));

	// Read last global ID
	xAttrib = aXNode->first_attribute("lastgid");
	if (xAttrib)
		lastGid = static_cast<uint32_t>(std::stoul(xAttrib->value()));

	mTilesets.emplace_back(TmxTileset(name, source, firstGid, lastGid));
}

void TmxMap::ReadLayer(rapidxml::xml_node<>* aXNode)
{
	std::string_view name;
	int width = 0, height = 0;

	// Read name
	auto xAttrib = aXNode->first_attribute("name");
	if (xAttrib != nullptr)
		name = xAttrib->value();

	// Read width
	xAttrib = aXNode->first_attribute("width");
	if (xAttrib != nullptr)
		width = std::stoi(xAttrib->value());

	// Read height
	xAttrib = aXNode->first_attribute("height");
	if (xAttrib != nullptr)
		height = std::stoi(xAttrib->value());

	// Read tile data
	auto xData = aXNode->first_node("data");
	if (xData == nullptr)
		return;

	// TODO: don't assume base64
	std::vector<uint32_t> tileDat(width * height);
	if (!Decode(tileDat, xData->value()))
		return;

	mLayers.emplace_back(TmxLayer(width, height, name, std::move(tileDat)));
}

void TmxMap::ReadObjects(rapidxml::xml_node<>* aXNode)
{
	for (auto xNode = aXNode->first_node(); xNode != nullptr; xNode = xNode->next_sibling())
	{
		if (strcmp(xNode->name(), "object") != 0)
			continue;

		std::string_view name;
		float x = 0.0f, y = 0.0f;

		// Read name
		auto xAttrib = xNode->first_attribute("name");
		if (xAttrib != nullptr)
			name = xAttrib->value();

		// Read X pos
		xAttrib = xNode->first_attribute("x");
		if (xAttrib != nullptr)
			x = std::stof(xAttrib->value());

		// Read Y pos
		xAttrib = xNode->first_attribute("y");
		if (xAttrib != nullptr)
			y = std::stof(xAttrib->value());

		mObjects.emplace_back(TmxObject(name, x, y));
	}
}

bool TmxMap::Load(const std::string_view inPath)
{
	// Read file into a buffer
	auto inFile = std::ifstream(inPath);
	std::stringstream buf;
	buf << inFile.rdbuf();
	std::string strXml = buf.str();
	buf.clear();

	// Parse document
	rapidxml::xml_document<> xDoc;
	xDoc.parse<0>(const_cast<char*>(strXml.c_str()));

	// Get map node
	auto xMap = xDoc.first_node("map");
	if (xMap == nullptr)
		return false;

	// Read map attribs
	rapidxml::xml_attribute<>* xAttrib = nullptr;
	if ((xAttrib = xMap->first_attribute("width")) != nullptr)
		mWidth = std::stoi(xAttrib->value());
	if ((xAttrib = xMap->first_attribute("height")) != nullptr)
		mHeight = std::stoi(xAttrib->value());

	// Read nodes
	for (auto xNode = xMap->first_node(); xNode != nullptr; xNode = xNode->next_sibling())
	{
		// Read layer nodes
		const auto xName = xNode->name();
		if (std::strcmp(xName, "layer") == 0)
			ReadLayer(xNode);
		else if (std::strcmp(xName, "tileset") == 0)
			ReadTileset(xNode);
		else if (std::strcmp(xName, "objectgroup") == 0)
			ReadObjects(xNode);
	}

	return true;
}
