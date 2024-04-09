/* tmxreader.cpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#include "tmxreader.hpp"
#include "tmxtileset.hpp"
#include "tmxobject.hpp"
#include "tmxlayer.hpp"
#include "base64.h"
#ifdef USE_ZLIB
# include <zlib.h>
#else
# include "gzip.hpp"
#endif
#include <rapidxml/rapidxml.hpp>
#include <optional>
#include <algorithm>
#include <ranges>
#include <sstream>
#include <fstream>


class TmxMap
{
	int mWidth = 0, mHeight = 0;

	std::vector<TmxLayer>   mLayers;
	std::vector<TmxTileset> mTilesets;
	std::vector<TmxObject>  mObjects;

	[[nodiscard]] bool Decode(std::span<uint32_t> out, const std::string_view base64);
	void ReadTileset(rapidxml::xml_node<>* aXNode);
	void ReadLayer(rapidxml::xml_node<>* aXNode);
	void ReadObjects(rapidxml::xml_node<>* aXNode);

public:
	[[nodiscard]] bool Load(const std::string_view inPath);

	constexpr std::pair<int, int> TileCount() const noexcept { return { mWidth, mHeight }; }
	constexpr const std::vector<TmxTileset>& Tilesets() const noexcept { return mTilesets; }
	constexpr const std::vector<TmxLayer>& Layers() const noexcept { return mLayers; }
};


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

TmxReader::Error TmxReader::Open(const std::string& inPath,
	const std::string_view graphicsName,
	const std::string_view paletteName,
	const std::string_view collisionName,
	const std::map<std::string, uint32_t>& objMapping)
{
	TmxMap map;
	if (!map.Load(inPath))
		return Error::LOAD_FAILED;

	using std::optional;
	using std::reference_wrapper;

	optional<reference_wrapper<const TmxLayer>> layerGfx;
	optional<reference_wrapper<const TmxLayer>> layerCls;
	optional<reference_wrapper<const TmxLayer>> layerPal;
	optional<reference_wrapper<std::vector<const TmxObject>>> objGroups;

	// Read layers
	for (const auto& layer : map.Layers())
	{
		auto name = layer.Name();
		//FIXME: no error reporting when a layer fails to load
		if (layer.Tiles().empty())
			continue;

		if (!layerGfx.has_value() && (graphicsName.empty() || name == graphicsName))  { layerGfx = layer; }
		if (!collisionName.empty() && !layerCls.has_value() && name == collisionName) { layerCls = layer; }
		if (!paletteName.empty() && !layerPal.has_value() && name == paletteName)     { layerPal = layer; }
		/*
		else if (!objMapping.empty() && layer->getType() == tmx::Layer::Type::Object)
		{
			objGroups.emplace_back(layer->getLayerAs<ObjectGroup>());
		}
		*/
	}

	// Check layers
	if (!layerGfx.has_value())
		return graphicsName.empty()
			? Error::NO_LAYERS
			: Error::GRAPHICS_NOTFOUND;
	if (!layerCls.has_value() && !collisionName.empty())
		return Error::GRAPHICS_NOTFOUND;
	if (!layerPal.has_value() && !paletteName.empty())
		return Error::PALETTE_NOTFOUND;

	// Read TMX map
	mSize = Size{ map.TileCount().first, map.TileCount().second };
	size_t numTiles = static_cast<size_t>(mSize.width) * static_cast<size_t>(mSize.height);

	// Read graphics layer
	mGraphics.reserve(numTiles);
	for (auto tmxTile : layerGfx.value().get().Tiles())
		mGraphics.emplace_back(Tile{ tmxTile & ~FLIP_MASK, static_cast<uint8_t>((tmxTile & FLIP_MASK) >> 28) });

	// Read optional layers
	if (layerPal.has_value())
	{
		std::vector<uint32_t> v;
		v.reserve(numTiles);
		for (auto tmxTile : layerPal.value().get().Tiles())
			v.emplace_back(tmxTile & ~FLIP_MASK);
		mPalette.emplace(v);
	}
	if (layerCls.has_value())
	{
		std::vector<uint32_t> v;
		v.reserve(numTiles);
		for (auto tmxTile : layerCls.value().get().Tiles())
			v.emplace_back(tmxTile & ~FLIP_MASK);
		mCollision.emplace(v);
	}

	// Read tilesets
	const auto& tilesets = map.Tilesets();
	mGidTable.reserve(tilesets.size());
	for (const auto& set : tilesets)
		mGidTable.emplace_back(set.GidRange());

	// Read objects
	if (!objMapping.empty())
	{
		std::vector<Object> v;
		for (const auto& tmxObj : objGroups.value().get())
		{
			auto it = objMapping.find(std::string(tmxObj.Name()));
			if (it == objMapping.end())
				continue;

			const auto& pos = tmxObj.Pos();
			Object obj;
			obj.id = it->second;
			obj.x = pos.x;
			obj.y = pos.y;

			v.emplace_back(obj);
		}
		/*
		for (const auto& group : objGroups)
		{
			const auto& tmxObjects = group.get().Objects();
			v.reserve(v.size() + tmxObjects.size());
			for (const auto& tmxObj : tmxObjects)
			{
				auto it = objMapping.find(tmxObj.getName());
				if (it == objMapping.end())
					continue;

				const auto& aabb = tmxObj.getAABB();
				Object obj;
				obj.id = it->second;
				obj.x = aabb.left;
				obj.y = aabb.top;

				v.emplace_back(obj);
			}
		}
		*/
		mObjects.emplace(v);
	}

	return Error::OK;
}

uint32_t TmxReader::LidFromGid(uint32_t aGid) const
{
	for (auto range : mGidTable)
	{
		if (aGid >= range.first && aGid <= range.second)
			return aGid - (range.first - 1);
	}
	return aGid;
}
