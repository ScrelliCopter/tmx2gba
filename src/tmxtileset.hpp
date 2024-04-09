/* tmxtileset.hpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#ifndef TMXTILESET_HPP
#define TMXTILESET_HPP

#include <string>
#include <string_view>
#include <cstdint>

class TmxTileset
{
	std::string mName, mSource;
	uint32_t mFirstGid = 0, mLastGid = 0;

public:
	TmxTileset(const std::string_view name, const std::string_view source, uint32_t firstGid, uint32_t lastGid)
		: mName(name), mSource(source), mFirstGid(firstGid), mLastGid(lastGid) {}

	[[nodiscard]] constexpr const std::string_view Name() const noexcept { return mName; }
	[[nodiscard]] constexpr const std::string_view Source() const noexcept { return mSource; }
	[[nodiscard]] constexpr const std::pair<uint32_t, uint32_t> GidRange() const noexcept { return { mFirstGid, mLastGid }; }
};

#endif//TMXTILESET_HPP
