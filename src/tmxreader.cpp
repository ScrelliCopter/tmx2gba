/* tmxreader.cpp

  Copyright (C) 2015-2022 a dinosaur

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

#include "tmxreader.hpp"
#include "tmxtileset.hpp"
#include "tmxobject.hpp"
#include "tmxlayer.hpp"
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <rapidxml/rapidxml.hpp>
#include <base64.h>
#include <miniz.h>


TmxReader::~TmxReader()
{
	// Delete old tilesets.
	for (auto pTileset : mTilesets)
		delete pTileset;
	mTilesets.clear();

	// Delete old layers.
	for (auto pLay : mLayers)
		delete pLay;
	mLayers.clear();
}


bool TmxReader::DecodeMap(uint32_t* aOut, size_t aOutSize, const std::string& aBase64Dat)
{
	// Cut leading & trailing whitespace (including newlines)
	auto beg = std::find_if_not(aBase64Dat.begin(), aBase64Dat.end(), ::isspace);
	if (beg == std::end(aBase64Dat))
		return false;
	auto end = std::find_if_not(aBase64Dat.rbegin(), aBase64Dat.rend(), ::isspace);
	std::size_t begOff = std::distance(aBase64Dat.begin(), beg);
	std::size_t endOff = std::distance(end, aBase64Dat.rend()) - begOff;
	auto trimmed = aBase64Dat.substr(begOff, endOff);

	// Decode base64 string.
	std::string decoded = base64_decode(trimmed);

	// Decompress compressed data.
	auto dstSize = static_cast<mz_ulong>(aOutSize);
	int res = uncompress(
		(unsigned char*)aOut,
		&dstSize,
		reinterpret_cast<const unsigned char*>(decoded.data()),
		static_cast<mz_ulong>(decoded.size()));
	decoded.clear();
	if (res < 0)
		return false;

	return true;
}

void TmxReader::ReadTileset(rapidxml::xml_node<>* aXNode)
{
	rapidxml::xml_attribute<>* xAttrib;

	const char*	name = "";
	const char*	source = "";
	uint32_t firstGid = 0;

	// Read name.
	xAttrib = aXNode->first_attribute("name");
	if (xAttrib != nullptr)
		name = xAttrib->value();

	// Read source.
	xAttrib = aXNode->first_attribute("source");
	if (xAttrib != nullptr)
		source = xAttrib->value();

	// Read first global ID.
	xAttrib = aXNode->first_attribute("firstgid");
	if (xAttrib != nullptr)
		firstGid = std::stoul(xAttrib->value());

	mTilesets.push_back(new TmxTileset(name, source, firstGid));
}

void TmxReader::ReadLayer(rapidxml::xml_node<>* aXNode)
{
	rapidxml::xml_attribute<>* xAttrib;
	const char* name    = "";
	int         width   = 0;
	int         height  = 0;
	uint32_t*   tileDat = nullptr;

	// Read name.
	xAttrib = aXNode->first_attribute("name");
	if (xAttrib != nullptr)
		name = xAttrib->value();

	// Read width.
	xAttrib = aXNode->first_attribute("width");
	if (xAttrib != nullptr)
		width = std::stoi(xAttrib->value());

	// Read height.
	xAttrib = aXNode->first_attribute("height");
	if (xAttrib != nullptr)
		height = std::stoi(xAttrib->value());

	// Read tile data.
	auto xData = aXNode->first_node("data");
	if (xData != nullptr)
	{
		// TODO: don't assume base64 & zlib.
		tileDat = new uint32_t[width * height];
		if (!DecodeMap(tileDat, width * height * sizeof(uint32_t), std::string(xData->value())))
		{
			delete[] tileDat;
			tileDat = nullptr;
		}
	}

	mLayers.push_back(new TmxLayer(width, height, name, tileDat));
}

void TmxReader::ReadObjects(rapidxml::xml_node<>* aXNode)
{
	for (auto xNode = aXNode->first_node(); xNode != nullptr; xNode = xNode->next_sibling())
	{
		if (strcmp(xNode->name(), "object") != 0)
			continue;

		rapidxml::xml_attribute<>* xAttrib;
		const char*	name = "";
		float x = 0.0f;
		float y = 0.0f;

		// Read name.
		xAttrib = xNode->first_attribute("name");
		if (xAttrib != nullptr)
			name = xAttrib->value();

		// Read X pos.
		xAttrib = xNode->first_attribute("x");
		if (xAttrib != nullptr)
			x = std::stof(xAttrib->value());

		// Read Y pos.
		xAttrib = xNode->first_attribute("y");
		if (xAttrib != nullptr)
			y = std::stof(xAttrib->value());

		mObjects.push_back(new TmxObject(name, x, y));
	}
}

void TmxReader::Open(std::istream& aIn)
{
	// Delete old tilesets.
	for (auto tileset : mTilesets)
		delete tileset;
	mTilesets.clear();

	// Delete old layers.
	for (auto layer : mLayers)
		delete layer;
	mLayers.clear();

	mGidTable.clear();

	// Read string into a buffer.
	std::stringstream buf;
	buf << aIn.rdbuf();
	std::string strXml = buf.str();
	buf.clear();

	// Parse document.
	rapidxml::xml_document<> xDoc;
	xDoc.parse<0>((char*)strXml.c_str());

	// Get map node.
	auto xMap = xDoc.first_node("map");
	if (xMap == nullptr)
		return;

	// Read map attribs.
	rapidxml::xml_attribute<>* xAttrib = nullptr;
	if ((xAttrib = xMap->first_attribute("width")) != nullptr)
		mWidth = std::stoi(xAttrib->value());
	if ((xAttrib = xMap->first_attribute("height")) != nullptr)
		mHeight = std::stoi(xAttrib->value());

	// Read nodes.
	for (auto xNode = xMap->first_node(); xNode != nullptr; xNode = xNode->next_sibling())
	{
		// Read layer nodes.
		if (strcmp(xNode->name(), "layer") == 0)
			ReadLayer(xNode);
		else
		if (strcmp(xNode->name(), "tileset") == 0)
			ReadTileset(xNode);
		else
		if (strcmp(xNode->name(), "objectgroup") == 0)
			ReadObjects(xNode);
	}

	// Generate global id table.
	for (auto tileset : mTilesets)
		mGidTable.push_back(tileset->GetFirstGid());
	std::sort(mGidTable.rbegin(), mGidTable.rend());
}

const TmxLayer* TmxReader::GetLayer(const std::string& aName) const
{
	for (auto layer : mLayers)
	{
		if (layer->GetName() == aName)
			return layer;
	}
	return nullptr;
}

uint32_t TmxReader::LidFromGid(uint32_t aGid)
{
	for (uint32_t first : mGidTable)
	{
		if (first <= aGid)
			return aGid - (first - 1);
	}
	return aGid;
}
