/* tmx2gba.cpp - Copyright (C) 2015-2022 a dinosaur (zlib, see COPYING.txt) */

#include "tmxreader.hpp"
#include "tmxlayer.hpp"
#include "tmxobject.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <algorithm>
#include <ultragetopt.h>


const std::string helpUsage = "Usage: tmx2gba [-h] [-f file] [-r offset] [-lyc name] [-p 0-15] [-m name;id] <-i inpath> <-o outpath>";
const std::string helpShort = "Run 'tmx2gba -h' to view all available options.";
const std::string helpFull = R"(
-h ------------ Display this help & command info.
-l <name> ----- Name of layer to use (default first layer in TMX).
-y <name> ----- Layer for palette mappings.
-c <name> ----- Output a separate 8bit collision map of the specified layer.
-r <offset> --- Offset tile indices (default 0).
-p <0-15> ----- Select which palette to use for 4-bit tilesets.
-m <name;id> -- Map an object name to an ID, will enable object exports.
-i <path> ----- Path to input TMX file.
-o <path> ----- Path to output files.
-f <file> ----- Specify a file to use for flags, will override any options specified on the command line.)";

struct Arguments
{
	bool help = false;
	std::string inPath, outPath;
	std::string layer, collisionlay, paletteLay;
	std::string flagFile;
	int offset = 0;
	int palette = 0;
	std::vector<std::string> objMappings;
	bool objExport = false;
};

void ParseArgs(int argc, char** argv, Arguments& p)
{
	int opt;
	optreset = 1;
	while ((opt = getopt(argc, argv, "hr:l:c:p:y:m:i:o:f:")) > 0)
	{
		switch (opt)
		{
		case ('h'):
			p.help = true;
			return;

		case ('l'): p.layer = optarg; break;
		case ('c'): p.collisionlay = optarg; break;
		case ('y'): p.paletteLay = optarg; break;
		case ('r'): p.offset = std::stoi(optarg); break;
		case ('p'): p.palette = std::stoi(optarg); break;

		case ('m'):
			p.objExport = true;
			p.objMappings.emplace_back(optarg);
			break;

		case ('i'): p.inPath = optarg; break;
		case ('o'): p.outPath = optarg; break;
		case ('f'): p.flagFile = optarg; break;

		default:
			break;
		}
	}
}

bool CheckArgs(const Arguments& params)
{
	// Check my paranoia.
	if (params.inPath.empty())
	{
		std::cerr << "No input file specified." << std::endl;
		std::cout << helpUsage << std::endl << helpShort << std::endl;
		return false;
	}
	if (params.outPath.empty())
	{
		std::cerr << "No output file specified." << std::endl;
		std::cout << helpUsage << std::endl << helpShort << std::endl;
		return false;
	}
	if (params.palette < 0 || params.palette > 15)
	{
		std::cerr << "Invalid palette index." << std::endl;
		std::cout << helpUsage << std::endl << helpShort << std::endl;
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
	ParseArgs(argc, argv, p);

	if (p.help)
	{
		std::cout << helpUsage << std::endl << helpFull << std::endl;
		return 0;
	}

	if (!p.flagFile.empty())
	{
		std::ifstream paramFile(p.flagFile);
		if (!paramFile.is_open())
		{
			std::cerr << "Failed to open param file." << std::endl;
			return -1;
		}
		
		std::vector<std::string> fileArgTokens;
		fileArgTokens.push_back("auu~~");
		bool carry = false;
		std::string rawToken;
		while (!paramFile.eof())
		{
			if (carry)
			{
				std::string tmp;
				paramFile >> tmp;
				rawToken += " ";
				rawToken += tmp;
			}
			else
			{
				rawToken.clear();
				paramFile >> rawToken;
			}

			if (rawToken.empty())
				continue;

			bool qFr = rawToken[0] == '"';
			bool qBk = rawToken[rawToken.length() - 1] == '"';
			if (qFr && qBk)
			{
				fileArgTokens.push_back(rawToken.substr(1, rawToken.length() - 2));
			}
			else
			if (qFr)
			{
				fileArgTokens.push_back(rawToken.substr(1, rawToken.length() - 1));
				carry = true;
			}
			else
			if (qBk)
			{
				fileArgTokens.push_back(rawToken.substr(0, rawToken.length() - 1));
				carry = false;
			}
			else
			{
				fileArgTokens.push_back(rawToken);
			}
		}

		std::vector<const char*> fileArgs;
		fileArgs.reserve(fileArgTokens.size());
		for (const auto& token : fileArgTokens)
			fileArgs.push_back(token.c_str());
		fileArgs.push_back(nullptr);

		ParseArgs(static_cast<int>(fileArgs.size()) - 1, (char**)fileArgs.data(), p);
	}

	if (!CheckArgs(p))
		return -1;

	// Object mappings.
	std::map<std::string, uint32_t> objMapping;
	if (p.objExport)
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

	// Open & read input file.
	TmxReader tmx;
	std::ifstream fin(p.inPath);
	if (!fin.is_open())
	{
		std::cerr << "Failed to open input file." << std::endl;
		return -1;
	}
	tmx.Open(fin);

	// Get layers.
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

	// Open output files.
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

	// Write header guards.
	std::string guard = "TMX2GBA_" + name;
	for (auto& c: guard)
		c = static_cast<char>(toupper(c));
	foutH << "#ifndef " << guard << std::endl;
	foutH << "#define " << guard << std::endl;
	foutH << std::endl;
	foutH << "#define " << name << "Width " << tmx.GetWidth() << std::endl;
	foutH << "#define " << name << "Height " << tmx.GetHeight() << std::endl;
	foutH << std::endl;

	// Convert to GBA-friendly charmap data.
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

		// Determine palette ID.
		uint32_t idx = 0;
		if (palTiles != nullptr)
			idx = tmx.LidFromGid((*palTiles++) & ~TmxLayer::FLIP_MASK);
		if (idx == 0)
			idx = p.palette + 1;
		flags |= static_cast<uint8_t>(idx - 1) << 4;

		charDat.push_back(tile | (static_cast<uint16_t>(flags) << 8));
	}

	// Save out charmap.
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

	// Convert collision map & save it out.
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

		// Try to nicely append "_collision" to the output name.
		std::string path;
		size_t extPos = p.outPath.find_last_of('.');
		if (extPos != std::string::npos)
			path = p.outPath.insert(extPos, "_collision");
		else
			path = p.outPath + "_collision";

		// Save it out.
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

	if (p.objExport)
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

		// Save it out.
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
