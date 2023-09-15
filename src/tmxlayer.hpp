/* tmxlayer.hpp - Copyright (C) 2015-2022 a dinosaur (zlib, see COPYING.txt) */

#ifndef TMXLAYER_H
#define TMXLAYER_H

#include <string>
#include <cstdint>
#include <utility>

class TmxLayer
{
public:
	static constexpr uint32_t FLIP_HORZ = 0x80000000;
	static constexpr uint32_t FLIP_VERT = 0x40000000;
	static constexpr uint32_t FLIP_DIAG = 0x20000000;
	static constexpr uint32_t FLIP_MASK = 0xE0000000;

	TmxLayer() : mWidth(0), mHeight(0), mTileDat(nullptr) {}
	TmxLayer(int aWidth, int aHeight, std::string aName, uint32_t* aTileDat)
		: mName(std::move(aName)), mWidth(aWidth), mHeight(aHeight), mTileDat(aTileDat) {}
	inline ~TmxLayer() { delete[] mTileDat; }

	constexpr const std::string& GetName() const { return mName; }
	constexpr int GetWidth() const { return mWidth; }
	constexpr int GetHeight() const { return mHeight; }
	constexpr const uint32_t* GetData() const { return mTileDat; }

private:
	std::string mName;
	int mWidth, mHeight;
	uint32_t* mTileDat;
};

#endif//TMXLAYER_H
