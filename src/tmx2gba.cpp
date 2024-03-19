/* tmx2gba.cpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#include "argparse.hpp"
#include "tmxreader.hpp"
#include "tmxlayer.hpp"
#include "tmxobject.hpp"
#include <iostream>
#include <fstream>
#include <map>
#include <cstdint>
#include <algorithm>


struct Arguments
{
	bool help = false;
	std::string inPath, outPath;
	std::string layer, collisionlay, paletteLay;
	std::string flagFile;
	int offset = 0;
	int palette = 0;
	std::vector<std::string> objMappings;
};

using ArgParse::Option;

static const ArgParse::Options options =
{
	Option::Optional('h', nullptr,   "Display this help & command info"),
	Option::Optional('l', "name",    "Name of layer to use (default first layer in TMX)"),
	Option::Optional('y', "name",    "Layer for palette mappings"),
	Option::Optional('c', "name",    "Output a separate 8bit collision map of the specified layer"),
	Option::Optional('r', "offset",  "Offset tile indices (default 0)"),
	Option::Optional('p', "0-15",    "Select which palette to use for 4-bit tilesets"),
	Option::Optional('m', "name;id", "Map an object name to an ID, will enable object exports"),
	Option::Required('i', "inpath",  "Path to input TMX file"),
	Option::Required('o', "outpath", "Path to output files"),
	Option::Optional('f', "file",    "Specify a file to use for flags, will override any options"
	                                 " specified on the command line")
};

bool ParseArgs(int argc, char** argv, Arguments& params)
{
	auto parser = ArgParse::ArgParser(argv[0], options, [&](int opt, const std::string_view arg)
		-> ArgParse::ParseCtrl
	{
		using ArgParse::ParseCtrl;
		try
		{
			switch (opt)
			{
			case 'h': params.help = true;        return ParseCtrl::QUIT_EARLY;
			case 'l': params.layer = arg;        return ParseCtrl::CONTINUE;
			case 'c': params.collisionlay = arg; return ParseCtrl::CONTINUE;
			case 'y': params.paletteLay = arg;   return ParseCtrl::CONTINUE;
			case 'r': params.offset = std::stoi(std::string(arg));  return ParseCtrl::CONTINUE;
			case 'p': params.palette = std::stoi(std::string(arg)); return ParseCtrl::CONTINUE;
			case 'm': params.objMappings.emplace_back(arg);         return ParseCtrl::CONTINUE;
			case 'i': params.inPath = arg;       return ParseCtrl::CONTINUE;
			case 'o': params.outPath = arg;      return ParseCtrl::CONTINUE;
			case 'f': params.flagFile = arg;     return ParseCtrl::CONTINUE;

			default: return ParseCtrl::QUIT_ERR_UNKNOWN;
			}
		}
		catch (std::invalid_argument const& e) { return ParseCtrl::QUIT_ERR_INVALID; }
		catch (std::out_of_range const& e) { return ParseCtrl::QUIT_ERR_RANGE; }
	});

	if (!parser.Parse(std::span(argv + 1, argc - 1)))
		return false;

	if (params.help)
		return true;

	if (!params.flagFile.empty())
	{
		std::ifstream paramFile(params.flagFile);
		if (!paramFile.is_open())
		{
			std::cerr << "Failed to open param file." << std::endl;
			return false;
		}

		std::vector<std::string> tokens;
		if (!ArgParse::ReadParamFile(tokens, paramFile))
		{
			std::cerr << "Failed to read param file: Unterminated quote string." << std::endl;
			return false;
		}

		if (!parser.Parse(tokens))
			return false;
	}

	// Check my paranoia
	if (params.inPath.empty())
	{
		parser.DisplayError("No input file specified.");
		return false;
	}
	if (params.outPath.empty())
	{
		parser.DisplayError("No output file specified.");
		return false;
	}
	if (params.palette < 0 || params.palette > 15)
	{
		parser.DisplayError("Invalid palette index.");
		return false;
	}

	return true;
}

template <typename T> constexpr const char* DatType();
template <> constexpr const char* DatType<uint8_t>() { return ".byte"; }
template <> constexpr const char* DatType<uint16_t>() { return ".hword"; }
template <> constexpr const char* DatType<uint32_t>() { return ".word"; }

template <typename T>
void WriteArray(std::ofstream& aOut, const std::vector<T>& aDat, int aPerCol = 16)
{
	int col = 0;

	aOut.setf(std::ios::hex, std::ios::basefield);
	aOut.setf(std::ios::showbase);

	size_t i = 0;
	for (T element : aDat)
	{
		if (col == 0)
			aOut << "\t" << DatType<T>() << " ";

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

int main(int argc, char** argv)
{
	Arguments p;
	if (!ParseArgs(argc, argv, p))
		return 1;
	if (p.help)
	{
		options.ShowHelpUsage(argv[0], std::cout);
		return 0;
	}

	// Object mappings
	std::map<std::string, uint32_t> objMapping;
	if (!p.objMappings.empty())
	{
		for (const auto& objToken : p.objMappings)
		{
			auto splitter = objToken.find_last_of(';');
			if (splitter == std::string::npos)
			{
				std::cerr << "Malformed mapping (missing a splitter)." << std::endl;
				return -1;
			}

			try
			{
				std::string name = objToken.substr(0, splitter);
				int id = std::stoi(objToken.substr(splitter + 1));

				objMapping[name] = id;
			}
			catch (std::exception&)
			{
				std::cerr << "Malformed mapping, make sure id is numeric." << std::endl;
			}
		}
	}

	// Open & read input file
	TmxReader tmx;
	std::ifstream fin(p.inPath);
	if (!fin.is_open())
	{
		std::cerr << "Failed to open input file." << std::endl;
		return -1;
	}
	tmx.Open(fin);

	// Get layers
	if (tmx.GetLayerCount() == 0)
	{
		std::cerr << "No layers found." << std::endl;
		return -1;
	}
	const TmxLayer* layerGfx = p.layer.empty()
		? tmx.GetLayer(0)
		: tmx.GetLayer(p.layer);
	const TmxLayer* layerCls = p.collisionlay.empty()
		? nullptr
		: tmx.GetLayer(p.collisionlay);
	const TmxLayer* layerPal = p.paletteLay.empty()
		? nullptr
		: tmx.GetLayer(p.paletteLay);

	if (layerGfx == nullptr)
	{
		std::cerr << "Input layer not found." << std::endl;
		return -1;
	}

	// Open output files
	std::ofstream foutS(p.outPath + ".s");
	std::ofstream foutH(p.outPath + ".h");
	if (!foutS.is_open() || !foutH.is_open())
	{
		std::cerr << "Failed to create output file(s).";
		return -1;
	}

	int slashPos = std::max((int)p.outPath.find_last_of('/'), (int)p.outPath.find_last_of('\\'));
	std::string name = p.outPath;
	if (slashPos != -1)
		name = name.substr(slashPos + 1);

	// Write header guards
	std::string guard = "TMX2GBA_" + name;
	for (auto& c: guard)
		c = static_cast<char>(toupper(c));
	foutH << "#ifndef " << guard << std::endl;
	foutH << "#define " << guard << std::endl;
	foutH << std::endl;
	foutH << "#define " << name << "Width " << tmx.GetWidth() << std::endl;
	foutH << "#define " << name << "Height " << tmx.GetHeight() << std::endl;
	foutH << std::endl;

	// Convert to GBA-friendly charmap data
	const uint32_t* gfxTiles = layerGfx->GetData();
	const uint32_t* palTiles = (layerPal == nullptr) ? nullptr : layerPal->GetData();
	std::vector<uint16_t> charDat;
	size_t numTiles = static_cast<size_t>(layerGfx->GetWidth()) * static_cast<size_t>(layerGfx->GetHeight());
	charDat.reserve(numTiles);
	for (size_t i = 0; i < numTiles; ++i)
	{
		uint32_t read = (*gfxTiles++);

		uint16_t tile = (uint16_t)std::max<int32_t>(0, tmx.LidFromGid(read & ~TmxLayer::FLIP_MASK) + p.offset);
		uint8_t flags = 0x0;

		// Get flipped!
		flags |= (read & TmxLayer::FLIP_HORZ) ? 0x4 : 0x0;
		flags |= (read & TmxLayer::FLIP_VERT) ? 0x8 : 0x0;

		// Determine palette ID
		uint32_t idx = 0;
		if (palTiles != nullptr)
			idx = tmx.LidFromGid((*palTiles++) & ~TmxLayer::FLIP_MASK);
		if (idx == 0)
			idx = p.palette + 1;
		flags |= static_cast<uint8_t>(idx - 1) << 4;

		charDat.push_back(tile | (static_cast<uint16_t>(flags) << 8));
	}

	// Write out charmap
	foutH << "#define " << name << "TilesLen " << charDat.size() * 2 << std::endl;
	foutH << "extern const unsigned short " << name << "Tiles[" << charDat.size() << "];" << std::endl;
	foutH << std::endl;

	foutS << "\t.section .rodata" << std::endl;
	foutS << "\t.align 2" << std::endl;
	foutS << "\t.global " << name << "Tiles" << std::endl;
	foutS << "\t.hidden " << name << "Tiles" << std::endl;
	foutS << name << "Tiles" << ":" << std::endl;
	WriteArray<uint16_t>(foutS, charDat);
	foutS << std::endl;

	// Convert collision map & write it out
	if (layerCls != nullptr)
	{
		std::vector<uint8_t> vucCollisionDat;
		vucCollisionDat.reserve(layerCls->GetWidth() * layerCls->GetHeight());

		gfxTiles = layerCls->GetData();
		for (int i = 0; i < layerCls->GetWidth() * layerCls->GetHeight(); ++i)
		{
			uint8_t ucTile = (uint8_t)tmx.LidFromGid((*gfxTiles++) & ~TmxLayer::FLIP_MASK);
			vucCollisionDat.push_back(ucTile);
		}

		// Try to nicely append "_collision" to the output name
		std::string path;
		size_t extPos = p.outPath.find_last_of('.');
		if (extPos != std::string::npos)
			path = p.outPath.insert(extPos, "_collision");
		else
			path = p.outPath + "_collision";

		// Write collision
		foutH << "#define " << name << "CollisionLen " << vucCollisionDat.size() << std::endl;
		foutH << "extern const unsigned char " << name << "Collision[" << vucCollisionDat.size() << "];" << std::endl;
		foutH << std::endl;

		foutS << std::endl;
		foutS << "\t.section .rodata" << std::endl;
		foutS << "\t.align 2" << std::endl;
		foutS << "\t.global " << name << "Collision" << std::endl;
		foutS << "\t.hidden " << name << "Collision" << std::endl;
		foutS << name << "Collision" << ":" << std::endl;
		WriteArray<uint8_t>(foutS, vucCollisionDat);
		foutS << std::endl;
	}

	if (!p.objMappings.empty())
	{
		std::vector<uint32_t> objDat;
		for (size_t i = 0; i < tmx.GetObjectCount(); ++i)
		{
			auto obj = tmx.GetObject(i);
			auto it = objMapping.find(obj->GetName());
			if (it == objMapping.end())
				continue;

			float x, y;
			obj->GetPos(x, y);
			objDat.push_back(it->second);
			objDat.push_back((int)(x * 256.0f));
			objDat.push_back((int)(y * 256.0f));
		}

		// Write objects
		foutH << "#define " << name << "ObjCount " << objDat.size() / 3 << std::endl;
		foutH << "#define " << name << "ObjdatLen " << objDat.size() * sizeof(int) << std::endl;
		foutH << "extern const unsigned int " << name << "Objdat[" << objDat.size() << "];" << std::endl;
		foutH << std::endl;

		foutS << std::endl;
		foutS << "\t.section .rodata" << std::endl;
		foutS << "\t.align 2" << std::endl;
		foutS << "\t.global " << name << "Objdat" << std::endl;
		foutS << "\t.hidden " << name << "Objdat" << std::endl;
		foutS << name << "Objdat" << ":" << std::endl;
		WriteArray<uint32_t>(foutS, objDat);
		foutS << std::endl;
	}

	foutH << "#endif//" << guard << std::endl;

	foutH.close();
	foutS.close();

	return 0;
}
