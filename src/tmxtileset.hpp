/* tmxtileset.hpp - Copyright (C) 2015-2022 a dinosaur (zlib, see COPYING.txt) */

#ifndef TMXTILESET_H
#define TMXTILESET_H

#include <string>
#include <cstdint>
#include <utility>

class TmxTileset
{
public:
	TmxTileset() : mFirstGid(0) {}
	TmxTileset(std::string aName, std::string aSource, uint32_t aFirstGid)
		: mName(std::move(aName)), mSource(std::move(aSource)), mFirstGid(aFirstGid) {}
	~TmxTileset() = default;

	constexpr const std::string& GetName() const { return mName; }
	constexpr const std::string& GetSource() const { return mSource; }
	constexpr uint32_t GetFirstGid() const { return mFirstGid; }

private:
	std::string mName;
	std::string mSource;
	uint32_t mFirstGid;

};

#endif//TMXTILESET_H
