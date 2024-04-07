// gzip.cpp - portable memory miniz based gzip reader
// SPDX-License-Identifier: Zlib
// SPDX-FileCopyrightText: (c) 2024 a dinosaur

#include "tmxlite/detail/gzip.hpp"
#include <string_view>


GZipReader::GZipReader() noexcept :
	mSourceLen(0), mBytesRead(0),
	mModificationTime(0), mCrc(0), mInputSize(0), mComputedCrc(0),
	crc16(0), mFlags(0), mXflags(0), mOsId(0)
{
	tinfl_init(&mState);
	mComputedCrc = static_cast<uint32_t>(mz_crc32(0, nullptr, 0));
}

bool GZipReader::OpenMemory(const std::span<const uint8_t> source) noexcept
{
	if (source.size() < 20)
		return false;

	auto it = std::cbegin(source), end = std::cend(source);

	constexpr uint8_t magic[2] = { 0x1F, 0x8B };
	if (*it++ != magic[0] || *it++ != magic[1])
		return false;

	constexpr uint8_t CM_DEFLATE = 8;
	uint8_t compression = *it++;
	if (compression != CM_DEFLATE)
		return false;

	mFlags = *it++;
	mModificationTime = *it++;
	mModificationTime |= *it++ << 8;
	mModificationTime |= *it++ << 16;
	mModificationTime |= *it++ << 24;
	mXflags = *it++;
	mOsId = *it++;

	if (mFlags & FEXTRA)
	{
		// Skip "extra" field
		if (it + 2 >= end)
			return false;
		uint16_t extraLen = *it++;
		extraLen = *it++ << 8;
		if (it + extraLen >= end)
			return false;
		it += extraLen;
	}
	if (mFlags & FNAME)
	{
		// Skip null-terminated name string
		do
		{
			if (++it == end)
				return false;
		} while (*it != '\0');
		if (++it == end)
			return false;
	}
	if (mFlags & FCOMMENT)
	{
		// Skip null-terminated comment string
		do
		{
			if (++it == end)
				return false;
		} while (*it != '\0');
		if (++it == end)
			return false;
	}
	if (mFlags & FHCRC)
	{
		if (it + 2 >= end)
			return false;
		crc16 = *it++;
		crc16 |= *it++;
	}

	mIt = it;
	mSourceLen = end - it - 8;

	it += mSourceLen;
	mCrc = *it++;
	mCrc |= *it++ << 8;
	mCrc |= *it++ << 16;
	mCrc |= *it++ << 24;
	mInputSize = *it++;
	mInputSize |= *it++ << 8;
	mInputSize |= *it++ << 16;
	mInputSize |= *it++ << 24;

	return true;
}

bool GZipReader::Read(std::span<uint8_t> out) noexcept
{
	size_t outLen = out.size();
	auto res = tinfl_decompress(&mState,
		static_cast<const mz_uint8*>(&*mIt), &mSourceLen,
		static_cast<mz_uint8*>(out.data()), static_cast<mz_uint8*>(out.data()), &outLen,
		TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
	if (res != TINFL_STATUS_DONE)
		return false;

	mIt += outLen;
	mBytesRead += outLen;
	mComputedCrc = static_cast<uint32_t>(mz_crc32(static_cast<mz_ulong>(mComputedCrc), out.data(), outLen));

	return true;
}

bool GZipReader::Check() const noexcept
{
	if (mComputedCrc != mCrc)
		return false;

	if (static_cast<uint32_t>(mBytesRead & UINT32_MAX) != mInputSize)
		return false;

	return true;
}
