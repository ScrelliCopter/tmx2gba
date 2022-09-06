/* tmxtileset.cpp

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
