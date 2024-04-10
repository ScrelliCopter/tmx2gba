// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2015-2024 a dinosaur

#ifndef HEADERWRITER_HPP
#define HEADERWRITER_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <span>
#include <concepts>
#include <fstream>
#include <filesystem>

template <typename T>
concept NumericType = std::integral<T> || std::floating_point<T>;

class HeaderWriter
{
	std::ofstream stream;
	std::string mName;

	void WriteGuardStart();
	void WriteGuardEnd();

public:
	~HeaderWriter();

	[[nodiscard]] bool Open(const std::filesystem::path& path, const std::string_view name);

	void WriteDefine(const std::string_view name, const std::string_view value);
	void WriteSymbol(const std::string_view name, const std::string_view type, std::size_t count);

	template <NumericType T>
	void WriteDefine(const std::string_view name, T value)
	{
		WriteDefine(name, std::to_string(value));
	}

	void WriteSize(int width, int height);
	void WriteCharacterMap(const std::span<uint16_t> charData);
	void WriteCollision(const std::span<uint8_t> collisionData);
	void WriteObjects(const std::span<uint32_t> objData);
};

#endif//HEADERWRITER_HPP
