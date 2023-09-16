/* tmxobject.hpp - Copyright (C) 2015-2022 a dinosaur (zlib, see COPYING.txt) */

#ifndef TMXOBJECT_HPP
#define TMXOBJECT_HPP

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

#endif//TMXOBJECT_HPP
