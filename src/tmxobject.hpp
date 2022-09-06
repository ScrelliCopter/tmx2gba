/* tmxobject.cpp

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

#ifndef TMXOBJECT_H
#define TMXOBJECT_H

#include <string>
#include <utility>

class TmxObject
{
public:
	TmxObject() : mX(0.0f), mY(0.0f) {}
	TmxObject(std::string aName, float aX, float aY)
		: mName(std::move(aName)), mX(aX), mY(aY) {}
	~TmxObject() = default;

	constexpr const std::string& GetName() const { return mName; }
	inline void GetPos(float& aOutX, float& aOutY) const { aOutX = mX; aOutY = mY; }

private:
	std::string mName;
	float mX, mY;
};

#endif//TMXOBJECT_H
