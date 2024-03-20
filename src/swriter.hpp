/* swwriter.hpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#ifndef SWRITER_HPP
#define SWRITER_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <fstream>
#include <vector>

namespace GccIsDumb
{
	template <typename T> constexpr const char* DatType();
	template <> constexpr const char* DatType<uint8_t>() { return ".byte"; }
	template <> constexpr const char* DatType<uint16_t>() { return ".hword"; }
	template <> constexpr const char* DatType<uint32_t>() { return ".word"; }
}

class SWriter
{
	std::ofstream stream;
	int writes = 0;

	template <typename T>
	static void WriteArray(std::ostream& aOut, const std::vector<T>& aDat, int aPerCol = 16)
	{
		int col = 0;

		aOut.setf(std::ios::hex, std::ios::basefield);
		aOut.setf(std::ios::showbase);

		size_t i = 0;
		for (T element : aDat)
		{
			if (col == 0)
				aOut << "\t" << GccIsDumb::DatType<T>() << " ";

			aOut << std::hex << (int)element;

			if (i < aDat.size() - 1)
			{
				if (++col < aPerCol)
				{
					aOut << ",";
				}
				else
				{
					aOut << "" << std::endl;
					col = 0;
				}
			}

			++i;
		}
	}

public:
	[[nodiscard]] bool Open(const std::filesystem::path& path)
	{
		stream.open(path);
		return stream.is_open();
	}

	~SWriter()
	{
		if (stream.is_open())
		{
			stream.close();
		}
	}

	template <typename T>
	void WriteArray(const std::string_view name, T data)
	{
		if (writes++ != 0)
			stream << std::endl;
		stream << "\t.section .rodata" << std::endl;
		stream << "\t.align 2" << std::endl;
		stream << "\t.global " << name << "Tiles" << std::endl;
		stream << "\t.hidden " << name << "Tiles" << std::endl;
		stream << name << "Tiles" << ":" << std::endl;
		WriteArray(stream, data);
		stream << std::endl;
	}
};

#endif//SWRITER_HPP
