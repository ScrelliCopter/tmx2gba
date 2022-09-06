/* tmxlayer.cpp

  Copyright (C) 2015-2022 a dinosaur

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

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
