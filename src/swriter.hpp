// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2024 a dinosaur

#ifndef SWRITER_HPP
#define SWRITER_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <span>
#include <fstream>
#include <filesystem>

class SWriter
{
	std::ofstream stream;
	std::string mName;
	int writes = 0;

	void WriteSymbol(const std::string_view suffix);

public:
	[[nodiscard]] bool Open(const std::filesystem::path& path, const std::string_view name);

	void WriteArray(const std::string_view suffix, std::span<uint8_t> data, int numCols = 16);
	void WriteArray(const std::string_view suffix, std::span<uint16_t> data, int numCols = 16);
	void WriteArray(const std::string_view suffix, std::span<uint32_t> data, int numCols = 16);
};

#endif//SWRITER_HPP
