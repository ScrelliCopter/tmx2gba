// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2024 a dinosaur

#ifndef CONVERT_HPP
#define CONVERT_HPP

#include <cstdint>
#include <vector>

class TmxReader;

namespace convert
{
	[[nodiscard]] bool ConvertCharmap(std::vector<uint16_t>& out,
		int idOffset, uint32_t defaultPalIdx,
		const TmxReader& tmx);
	[[nodiscard]] bool ConvertCollision(std::vector<uint8_t>& out, const TmxReader& tmx);
	[[nodiscard]] bool ConvertObjects(std::vector<uint32_t>& out, const TmxReader& tmx);
};

#endif//CONVERT_HPP
