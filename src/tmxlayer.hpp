// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#ifndef TMXLAYER_HPP
#define TMXLAYER_HPP

#include <vector>
#include <span>
#include <string>
#include <string_view>
#include <cstdint>
#include <utility>

class TmxLayer
{
	std::string mName;
	int mWidth, mHeight;
	std::vector<uint32_t> mTileDat;

public:
	static constexpr uint32_t FLIP_HORZ = 0x80000000;
	static constexpr uint32_t FLIP_VERT = 0x40000000;
	static constexpr uint32_t FLIP_DIAG = 0x20000000;
	static constexpr uint32_t FLIP_MASK = 0xE0000000;

	TmxLayer(int width, int height, const std::string_view name, std::vector<uint32_t>&& tileDat) noexcept
		: mName(name), mWidth(width), mHeight(height), mTileDat(std::move(tileDat)) {}

	[[nodiscard]] const std::string_view Name() const noexcept { return mName; }
	[[nodiscard]] constexpr std::pair<int, int> TileCount() const noexcept { return { mWidth, mHeight }; }
	[[nodiscard]] constexpr const std::span<const uint32_t> Tiles() const noexcept { return mTileDat; }
};

#endif//TMXLAYER_HPP
