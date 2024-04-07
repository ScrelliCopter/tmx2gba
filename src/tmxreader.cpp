/* tmxreader.cpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#include "tmxreader.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include <optional>
#include <algorithm>


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
	mSize = Size { map.getTileCount().x, map.getTileCount().y };
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
