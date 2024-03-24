/* converter.hpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#include "convert.hpp"
#include "tmxreader.hpp"


bool convert::ConvertCharmap(std::vector<uint16_t>& out, int idxOffset, uint32_t defaultPal, const TmxReader& tmx)
{
	const auto gfxTiles = tmx.GetGraphicsTiles();
	const auto palTiles = tmx.GetPaletteTiles();

	const size_t numTiles = tmx.TileCount();
	out.reserve(numTiles);
	for (size_t i = 0; i < numTiles; ++i)
	{
		const TmxReader::Tile tile = gfxTiles[i];

		int tileIdx = std::max(0, static_cast<int>(tmx.LidFromGid(tile.id)) + idxOffset);
		uint8_t flags = 0x0;

		// Get flipped!
		flags |= (tile.flags & TmxReader::FLIP_HORZ) ? 0x4 : 0x0;
		flags |= (tile.flags & TmxReader::FLIP_VERT) ? 0x8 : 0x0;

		// Determine palette ID
		uint32_t idx = 0;
		if (palTiles.has_value())
			idx = tmx.LidFromGid(palTiles.value()[i]);
		if (idx == 0)
			idx = defaultPal + 1;
		flags |= static_cast<uint8_t>(idx - 1) << 4;

		out.push_back(static_cast<uint16_t>(tileIdx) | static_cast<uint16_t>(flags << 8));
	}

	return true;
}

bool convert::ConvertCollision(std::vector<uint8_t>& out, const TmxReader& tmx)
{
	const auto clsTiles = tmx.GetCollisionTiles().value();

	size_t numTiles = tmx.TileCount();
	out.reserve(numTiles);
	for (size_t i = 0; i < numTiles; ++i)
	{
		uint8_t id = static_cast<uint8_t>(tmx.LidFromGid(clsTiles[i]));
		out.emplace_back(id);
	}

	return true;
}


bool convert::ConvertObjects(std::vector<uint32_t>& out, const TmxReader& tmx)
{
	const auto objects = tmx.GetObjects().value();

	for (auto obj : objects)
	{
		out.push_back(obj.id);
		out.emplace_back(static_cast<int>(obj.x * 256.0f));
		out.emplace_back(static_cast<int>(obj.y * 256.0f));
	}

	return true;
}
