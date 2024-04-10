// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#ifndef TMXREADER_HPP
#define TMXREADER_HPP

#include <string>
#include <string_view>
#include <cstdint>
#include <span>
#include <vector>
#include <map>
#include <optional>

class TmxReader
{
public:
	static constexpr uint8_t FLIP_HORZ = 0x8;
	static constexpr uint8_t FLIP_VERT = 0x4;
	static constexpr uint8_t FLIP_DIAG = 0x2;
	static constexpr uint8_t FLIP_MASK = 0xE;

	enum class Error
	{
		OK,
		LOAD_FAILED,
		NO_LAYERS,
		GRAPHICS_NOTFOUND,
		PALETTE_NOTFOUND,
		COLLISION_NOTFOUND
	};

	[[nodiscard]] Error Open(const std::string& inPath,
		const std::string_view graphicsName,
		const std::string_view paletteName,
		const std::string_view collisionName,
		const std::map<std::string, uint32_t>& objMapping);
	struct Size { int width, height; };

	[[nodiscard]] constexpr Size GetSize() const { return mSize; }
	[[nodiscard]] constexpr size_t TileCount() const { return
		static_cast<size_t>(mSize.width) *
		static_cast<size_t>(mSize.height); }

	[[nodiscard]] uint32_t LidFromGid(uint32_t aGid) const;

	struct Tile { uint32_t id; uint8_t flags; };
	struct Object { unsigned id; float x, y; };

	[[nodiscard]] constexpr bool HasCollisionTiles() const { return mCollision.has_value(); }
	[[nodiscard]] constexpr bool HasObjects() const { return mObjects.has_value(); }

	[[nodiscard]] constexpr const std::span<const Tile> GetGraphicsTiles() const { return mGraphics; }
	[[nodiscard]] constexpr const std::optional<std::span<const uint32_t>> GetPaletteTiles() const
	{
		if (mPalette.has_value()) { return { mPalette.value() }; }
		return std::nullopt;
	}
	[[nodiscard]] constexpr const std::optional<std::span<const uint32_t>> GetCollisionTiles() const
	{
		if (mCollision.has_value()) { return { mCollision.value() }; }
		return std::nullopt;
	}
	[[nodiscard]] constexpr const std::optional<std::span<const Object>> GetObjects() const
	{
		if (mObjects.has_value()) { return { mObjects.value() }; }
		return std::nullopt;
	}

private:
	Size mSize;

	std::vector<std::pair<uint32_t, uint32_t>> mGidTable;

	std::vector<Tile> mGraphics;
	std::optional<std::vector<uint32_t>> mPalette;
	std::optional<std::vector<uint32_t>> mCollision;
	std::optional<std::vector<Object>> mObjects;
};

#endif//TMXREADER_HPP
