/* tmx2gba.cpp - Copyright (C) 2015-2024 a dinosaur (zlib, see COPYING.txt) */

#include "argparse.hpp"
#include "tmxreader.hpp"
#include "convert.hpp"
#include "headerwriter.hpp"
#include "swriter.hpp"
#include "config.h"
#include <iostream>
#include <map>
#include <algorithm>


struct Arguments
{
	std::string inPath, outPath;
	std::string layer, collisionlay, paletteLay;
	std::string flagFile;
	int offset = 0;
	int palette = 0;
	std::vector<std::string> objMappings;
	bool help = false, showVersion = false;
};

using ArgParse::Option;

static const ArgParse::Options options =
{
	Option::Optional('h', nullptr,   "Display this help & command info"),
	Option::Optional('v', nullptr,   "Display version & quit"),
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

static bool ParseArgs(int argc, char** argv, Arguments& params)
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
			case 'v': params.showVersion = true; return ParseCtrl::QUIT_EARLY;
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
		catch (std::invalid_argument const&) { return ParseCtrl::QUIT_ERR_INVALID; }
		catch (std::out_of_range const&) { return ParseCtrl::QUIT_ERR_RANGE; }
	});

	if (!parser.Parse(std::span(argv + 1, argc - 1)))
		return false;

	if (params.help || params.showVersion)
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


static std::string SanitiseLabel(const std::string_view ident)
{
	std::string out;
	out.reserve(ident.length());

	int last = '_';
	for (int i : ident)
	{
		if (out.empty() && std::isdigit(i))
			continue;
		if (!std::isalnum(i))
			i = '_';
		if (i != '_' || last != '_')
			out.push_back(i);
		last = i;
	}
	return out;
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
	if (p.showVersion)
	{
		std::cout << "tmx2gba version " << TMX2GBA_VERSION << ", (c) 2015-2024 a dinosaur" << std::endl;
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
				return 1;
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
	switch (tmx.Open(p.inPath,
		p.layer, p.paletteLay, p.collisionlay, objMapping))
	{
	case TmxReader::Error::LOAD_FAILED:
		std::cerr << "Failed to open input file." << std::endl;
		return 1;
	case TmxReader::Error::NO_LAYERS:
		std::cerr << "No suitable tile layer found." << std::endl;
		return 1;
	case TmxReader::Error::GRAPHICS_NOTFOUND:
		std::cerr << "No graphics layer \"" << p.layer << "\" found." << std::endl;
		return 1;
	case TmxReader::Error::PALETTE_NOTFOUND:
		std::cerr << "No palette layer \"" << p.paletteLay << "\" found." << std::endl;
		return 1;
	case TmxReader::Error::COLLISION_NOTFOUND:
		std::cerr << "No collision layer \"" << p.collisionlay << "\" found." << std::endl;
		return 1;
	case TmxReader::Error::OK:
		break;
	}

	// Get name from file
	std::string name = SanitiseLabel(std::filesystem::path(p.outPath).stem().string());

	// Open output files
	SWriter outS;
	if (!outS.Open(p.outPath + ".s", name))
	{
		std::cerr << "Failed to create output file \"" << p.outPath << ".s\".";
		return 1;
	}
	HeaderWriter outH;
	if (!outH.Open(p.outPath + ".h", name))
	{
		std::cerr << "Failed to create output file \"" << p.outPath << ".h\".";
		return 1;
	}

	// Convert to GBA-friendly charmap data
	{
		std::vector<uint16_t> charDat;
		if (!convert::ConvertCharmap(charDat, p.offset, p.palette, tmx))
			return 1;

		// Write out charmap
		outH.WriteSize(tmx.GetSize().width, tmx.GetSize().height);
		outH.WriteCharacterMap(charDat);
		outS.WriteArray("Tiles", charDat);
	}

	// Convert collision map & write out
	if (tmx.HasCollisionTiles())
	{
		std::vector<uint8_t> collisionDat;
		if (!convert::ConvertCollision(collisionDat, tmx))
			return 1;

		outH.WriteCollision(collisionDat);
		outS.WriteArray("Collision", collisionDat, 32);
	}

	if (tmx.HasObjects())
	{
		std::vector<uint32_t> objDat;
		if (!convert::ConvertObjects(objDat, tmx))
			return 1;

		outH.WriteObjects(objDat);
		outS.WriteArray("Objdat", objDat);
	}

	return 0;
}
