// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#include "tmxreader.hpp"
#include "tmxmap.hpp"
#include <optional>
#include <algorithm>
#include <ranges>


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

	optional<reference_wrapper<const TmxLayer>> layerGfx, layerCls, layerPal;

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
		mGraphics.emplace_back(Tile{ tmxTile & ~TmxLayer::FLIP_MASK, static_cast<uint8_t>((tmxTile & TmxLayer::FLIP_MASK) >> 28) });

	// Read optional layers
	if (layerPal.has_value())
	{
		std::vector<uint32_t> v;
		v.reserve(numTiles);
		for (auto tmxTile : layerPal.value().get().Tiles())
			v.emplace_back(tmxTile & ~TmxLayer::FLIP_MASK);
		mPalette.emplace(v);
	}
	if (layerCls.has_value())
	{
		std::vector<uint32_t> v;
		v.reserve(numTiles);
		for (auto tmxTile : layerCls.value().get().Tiles())
			v.emplace_back(tmxTile & ~TmxLayer::FLIP_MASK);
		mCollision.emplace(v);
	}

	// Read tilesets
	const auto& tilesets = map.Tilesets();
	mGidTable.reserve(tilesets.size());
	std::ranges::transform(tilesets, std::back_inserter(mGidTable),
		[](const auto& it) { return it.GidRange(); });

	// Read objects
	if (!map.ObjectGroups().empty())
	{
		std::vector<Object> objs;
		for (const auto& group : map.ObjectGroups())
		{
			const auto& tmxObjects = group.Objects();
			objs.reserve(objs.size() + tmxObjects.size());
			for (const auto& tmxObj : tmxObjects)
			{
				auto it = objMapping.find(std::string(tmxObj.Name()));
				if (it == objMapping.end())
					continue;

				const auto& aabb = tmxObj.Box();
				objs.emplace_back(Object
				{
					.id = it->second,
					.x = aabb.x,
					.y = aabb.y
				});
			}
		}
		mObjects.emplace(objs);
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
