// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#ifndef TMXOBJECT_HPP
#define TMXOBJECT_HPP

#include <string>
#include <string_view>
#include <vector>
#include <utility>

class TmxObject
{
public:
	template <typename T>
	struct AABB { T x, y, w, h; };

	TmxObject(int id, std::string_view name, AABB<float>&& box) : mId(id), mName(name), mBox(std::move(box)) {}

	constexpr int Id() const noexcept { return mId; }
	const std::string_view Name() const noexcept { return mName; }
	constexpr const AABB<float>& Box() const noexcept { return mBox; }

private:
	int mId;
	std::string mName;
	AABB<float> mBox;
};

class TmxObjectGroup
{
	std::string mName;
	std::vector<TmxObject> mObjects;

public:
	TmxObjectGroup(std::string_view name, std::vector<TmxObject>&& objects)
		: mName(name), mObjects(std::move(objects)) {}

	const std::string_view Name() const noexcept { return mName; }
	constexpr const std::vector<TmxObject>& Objects() const noexcept { return mObjects; }
};

#endif//TMXOBJECT_HPP
