// gzip.hpp - portable memory miniz based gzip reader
// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2024 a dinosaur

#ifndef GZIP_HPP
#define GZIP_HPP

#include "miniz.h"
#include <cstdint>
#include <span>


class GZipReader
{
	static constexpr uint8_t
		FTEXT = 1, FHCRC = 1<<1, FEXTRA = 1<<2, FNAME = 1<<3, FCOMMENT = 1<<4;

	static constexpr uint8_t XFL_BEST = 2, XFL_FASTEST = 4;

	tinfl_decompressor mState;
	std::span<const uint8_t>::iterator mIt;

	size_t mSourceLen, mBytesRead;
	uint32_t mModificationTime, mCrc, mInputSize, mComputedCrc;
	uint16_t crc16;
	uint8_t mFlags, mXflags, mOsId;

public:
	GZipReader() noexcept;

	constexpr size_t SourceLength() const noexcept { return mSourceLen; }
	constexpr uint32_t OutputLength() const noexcept { return mInputSize; }

	bool OpenMemory(const std::span<const uint8_t> source) noexcept;
	bool Read(std::span<uint8_t> out) noexcept;
	bool Check() const noexcept;
};

#endif//GZIP_HPP
