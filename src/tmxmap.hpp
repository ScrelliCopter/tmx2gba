// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#ifndef TMXMAP_HPP
#define TMXMAP_HPP

#include "tmxtileset.hpp"
#include "tmxobject.hpp"
#include "tmxlayer.hpp"
#include <rapidxml/rapidxml.hpp>
#include <vector>
#include <span>
#include <string_view>

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

#endif//TMXMAP_HPP
