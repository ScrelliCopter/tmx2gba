/* tmxreader.cpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#include "tmxreader.hpp"
#include "tmxtileset.hpp"
#include "tmxobject.hpp"
#include "tmxlayer.hpp"
#include <optional>
#include <algorithm>

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

	// Decode base64 string
	std::string decoded = base64_decode(trimmed);

	// Decompress compressed data
	auto dstSize = static_cast<mz_ulong>(aOutSize);
	int res = uncompress(
		reinterpret_cast<unsigned char*>(aOut),
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

	// Read name
	xAttrib = aXNode->first_attribute("name");
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

	mTilesets.push_back(new TmxTileset(name, source, firstGid));
}

void TmxReader::ReadLayer(rapidxml::xml_node<>* aXNode)
{
	rapidxml::xml_attribute<>* xAttrib;
	const char* name    = "";
	int         width   = 0;
	int         height  = 0;
	uint32_t*   tileDat = nullptr;

	// Read name
	xAttrib = aXNode->first_attribute("name");
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
	if (xData != nullptr)
	{
		// TODO: don't assume base64 & zlib
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

		// Read name
		xAttrib = xNode->first_attribute("name");
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

		mObjects.push_back(new TmxObject(name, x, y));
	}
}

void TmxReader::Open(std::istream& aIn)
{
	// Delete old tilesets
	for (auto tileset : mTilesets)
		delete tileset;
	mTilesets.clear();

	// Delete old layers
	for (auto layer : mLayers)
		delete layer;
	mLayers.clear();

	mGidTable.clear();

	// Read string into a buffer
	std::stringstream buf;
	buf << aIn.rdbuf();
	std::string strXml = buf.str();
	buf.clear();

	// Parse document
	rapidxml::xml_document<> xDoc;
	xDoc.parse<0>(const_cast<char*>(strXml.c_str()));

	// Get map node
	auto xMap = xDoc.first_node("map");
	if (xMap == nullptr)
		return;

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
		if (strcmp(xNode->name(), "layer") == 0)
			ReadLayer(xNode);
		else
		if (strcmp(xNode->name(), "tileset") == 0)
			ReadTileset(xNode);
		else
		if (strcmp(xNode->name(), "objectgroup") == 0)
			ReadObjects(xNode);
	}

	// Generate global id table
	for (auto tileset : mTilesets)
		mGidTable.push_back(tileset->GetFirstGid());
	std::sort(mGidTable.rbegin(), mGidTable.rend());
}

TmxReader::Error TmxReader::Open(const std::string& inPath,
	const std::string_view graphicsName,
	const std::string_view paletteName,
	const std::string_view collisionName,
	const std::map<std::string, uint32_t>& objMapping)
{
	tmx::Map map;
	if (!map.load(inPath))
		return Error::LOAD_FAILED;

	using tmx::TileLayer;
	using tmx::ObjectGroup;
	using std::optional;
	using std::reference_wrapper;

	optional<reference_wrapper<const TileLayer>> layerGfx;
	optional<reference_wrapper<const TileLayer>> layerCls;
	optional<reference_wrapper<const TileLayer>> layerPal;
	std::vector<reference_wrapper<const ObjectGroup>> objGroups;

	// Read layers
	for (const auto& layer : map.getLayers())
	{
		auto name = layer->getName();
		if (layer->getType() == tmx::Layer::Type::Tile)
		{
			const auto& tileLayer = layer->getLayerAs<TileLayer>();
			// tmxlite unfortunately has no error reporting when a layer fails to load,
			//  empty check will suffice for the time being
			if (tileLayer.getTiles().empty())
				continue;

			if (layerGfx == std::nullopt && (graphicsName.empty() || name == graphicsName))
				layerGfx = tileLayer;
			if (!collisionName.empty() && layerCls == std::nullopt && name == collisionName)
				layerCls = tileLayer;
			if (!paletteName.empty() && layerPal == std::nullopt && name == paletteName)
				layerPal = tileLayer;
		}
		else if (!objMapping.empty() && layer->getType() == tmx::Layer::Type::Object)
		{
			objGroups.emplace_back(layer->getLayerAs<ObjectGroup>());
		}
	}

	// Check layers
	if (layerGfx == std::nullopt)
	{
		if (graphicsName.empty())
			return Error::NO_LAYERS;
		else
			return Error::GRAPHICS_NOTFOUND;
	}
	if (layerCls == std::nullopt && !collisionName.empty())
		return Error::GRAPHICS_NOTFOUND;
	if (layerPal == std::nullopt && !paletteName.empty())
		return Error::PALETTE_NOTFOUND;

	// Read TMX map
	mSize = Size{ map.getTileCount().x, map.getTileCount().y };
	size_t numTiles = static_cast<size_t>(mSize.width) * static_cast<size_t>(mSize.height);

	// Read graphics layer
	mGraphics.reserve(numTiles);
	for (auto tmxTile : layerGfx.value().get().getTiles())
		mGraphics.emplace_back(Tile { tmxTile.ID, tmxTile.flipFlags });

	// Read optional layers
	if (layerPal.has_value())
	{
		std::vector<uint32_t> v;
		v.reserve(numTiles);
		for (auto tmxTile : layerPal.value().get().getTiles())
			v.emplace_back(tmxTile.ID);
		mPalette.emplace(v);
	}
	if (layerCls.has_value())
	{
		std::vector<uint32_t> v;
		v.reserve(numTiles);
		for (auto tmxTile : layerCls.value().get().getTiles())
			v.emplace_back(tmxTile.ID);
		mCollision.emplace(v);
	}

	// Read tilesets
	const auto& tilesets = map.getTilesets();
	mGidTable.reserve(tilesets.size());
	for (const auto& set : tilesets)
		mGidTable.emplace_back(std::make_pair(set.getFirstGID(), set.getLastGID()));

	// Read objects
	if (!objMapping.empty())
	{
		std::vector<Object> v;
		for (const auto& group : objGroups)
		{
			const auto& tmxObjects = group.get().getObjects();
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
