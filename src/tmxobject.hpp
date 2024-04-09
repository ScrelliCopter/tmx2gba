/* tmxobject.hpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#ifndef TMXOBJECT_HPP
#define TMXOBJECT_HPP

#include <string>
#include <utility>

class TmxObject
{
public:
	TmxObject(std::string_view name, float x, float y) : mName(name), mPos{ x, y } {}

	template <typename T>
	struct Position { T x, y; };

	constexpr const std::string_view Name() const noexcept { return mName; }
	constexpr Position<float> Pos() const noexcept { return mPos; }

private:
	std::string mName;
	Position<float> mPos;
};

#endif//TMXOBJECT_HPP
