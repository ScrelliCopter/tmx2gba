// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#ifndef TMXMAP_HPP
#define TMXMAP_HPP

#include "tmxtileset.hpp"
#include "tmxobject.hpp"
#include "tmxlayer.hpp"
#include <vector>
#include <span>
#include <string>
#include <string_view>

namespace pugi { class xml_node; }

class TmxMap
{
	int mWidth = 0, mHeight = 0;

	std::vector<TmxLayer>       mLayers;
	std::vector<TmxTileset>     mTilesets;
	std::vector<TmxObjectGroup> mObjectGroups;

	void ReadTileset(const pugi::xml_node& xNode);
	void ReadLayer(const pugi::xml_node& xNode);
	void ReadObjectGroup(const pugi::xml_node& xNode);

public:
	[[nodiscard]] bool Load(const std::string& inPath);

	[[nodiscard]] constexpr std::pair<int, int> TileCount() const noexcept { return { mWidth, mHeight }; }
	[[nodiscard]] constexpr const std::vector<TmxTileset>& Tilesets() const noexcept { return mTilesets; }
	[[nodiscard]] constexpr const std::vector<TmxLayer>& Layers() const noexcept { return mLayers; }
	[[nodiscard]] constexpr const std::vector<TmxObjectGroup>& ObjectGroups() const noexcept { return mObjectGroups; }
};

#endif//TMXMAP_HPP
