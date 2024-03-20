/* headerwriter.cpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#include "headerwriter.hpp"


template <typename T> static constexpr std::string_view DatType();
template <> constexpr std::string_view DatType<uint8_t>() { return "unsigned char"; }
template <> constexpr std::string_view DatType<uint16_t>() { return "unsigned short"; }
template <> constexpr std::string_view DatType<uint32_t>() { return "unsigned int"; }

void HeaderWriter::WriteSize(int width, int height)
{
	stream << std::endl;
	WriteDefine(name + "Width", width);
	WriteDefine(name + "Height", height);
}

void HeaderWriter::WriteCharacterMap(const std::span<uint16_t> charData)
{
	stream << std::endl;
	WriteDefine(name + "TilesLen", charData.size() * 2);
	WriteSymbol(name + "Tiles", DatType<uint16_t>(), charData.size());
}

void HeaderWriter::WriteCollision(const std::span<uint8_t> collisionData)
{
	stream << std::endl;
	WriteDefine(name + "CollisionLen", collisionData.size());
	WriteSymbol(name + "Collision", DatType<uint8_t>(), collisionData.size());
}

void HeaderWriter::WriteObjects(const std::span<uint32_t> objData)
{
	stream << std::endl;
	WriteDefine(name + "ObjCount", objData.size() / 3);
	WriteDefine(name + "ObjdatLen", objData.size() * sizeof(int));
	WriteSymbol(name + "Objdat", DatType<uint32_t>(), objData.size());
}


static std::string GuardName(const std::string_view name)
{
	auto upper = std::string(name);
	for (auto& c: upper)
		c = static_cast<char>(toupper(c));
	return "TMX2GBA_" + upper;
}


void HeaderWriter::WriteGuardStart()
{
	const std::string guard = GuardName(name);
	stream << "#ifndef " << guard << std::endl;
	stream << "#define " << guard << std::endl;
}

void HeaderWriter::WriteGuardEnd()
{
	const std::string guard = GuardName(name);
	stream << std::endl << "#endif//" << guard << std::endl;
}


HeaderWriter::~HeaderWriter()
{
	if (stream.is_open())
	{
		WriteGuardEnd();
		stream.close();
	}
}


bool HeaderWriter::Open(const std::string_view path, const std::string_view name)
{
	this->name = name;
	stream.open(path);
	if (!stream.is_open())
		return false;

	WriteGuardStart();
	return true;
}

void HeaderWriter::WriteDefine(const std::string_view name, const std::string_view value)
{
	stream << "#define " << name << " " << value << std::endl;
}

void HeaderWriter::WriteSymbol(const std::string_view name, const std::string_view type, std::size_t count)
{
	stream << "extern const " << type << " " << name << "[" << count << "];" << std::endl;
}
