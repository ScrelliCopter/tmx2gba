/* tmx2gba.cpp

  Copyright (C) 2015 Nicholas Curtis

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <algorithm>
#include <rapidxml/rapidxml.hpp>
#include <base64.h>
#include <miniz.h>
#include <XGetopt.h>

static const std::string g_strUsage = "Usage: tmx2gba [-h] [-r offset] [-lc name] [-p 0-15] <-i inpath> <-o outpath>\nRun 'tmx2gba -h' to view all available options.";
static const std::string g_strFullHelp = R"(Usage: tmx2gba [-hr] [-p index] <-i inpath> <-o outpath>

-h ---------- Display this help & command info.
-l <name> --- Name of layer to use (default first layer in TMX).
-c <name> --- Output a separate 8bit collision map of the specified layer.
-r <offset> - Offset tile indices (default 0).
-p <0-15> --- Select which palette to use for 4-bit tilesets.
-i <path> --- Path to input TMX file.
-o <path> --- Path to output files.)";

struct STilemapInfo
{
	uint32_t width;
	uint32_t height;
};


bool DecodeMapData (
		const std::string& a_strEncData,
		int a_iWidth, int a_iHeight,
		std::vector<uint8_t>* a_pvucOut
	)
{
	if ( a_pvucOut == nullptr )
	{
		return false;
	}

	// Decode base64 string.
	unsigned int cutTheCrap = a_strEncData.find_first_not_of ( " \t\n\r" );
	std::string strDec = base64_decode ( a_strEncData.substr ( cutTheCrap ) );
	a_pvucOut->clear ();
	a_pvucOut->resize ( a_iWidth * a_iHeight * 4 );
	mz_ulong uiDstSize = a_pvucOut->size ();

	// Decompress compressed data.
	int iRes = uncompress (
			a_pvucOut->data (), &uiDstSize,
			(const uint8_t*)strDec.data (), strDec.size ()
		);
	strDec.clear ();
	if ( iRes < 0 )
	{
		return false;
	}

	return true;
}

int main ( int argc, char** argv )
{
	// Parse cmd line args.
	std::string strInPath, strOutPath;
	std::string strLayer, strCollisionlay;
	int iOffset = 0;
	int iPalette = 0;

	char cOption;
	while ( ( cOption = getopt ( argc, argv, "hr:l:c:p:i:o:" ) ) > 0 )
	{
		switch ( cOption )
		{
		case ( 'h' ):
			std::cout << g_strFullHelp << std::endl;
			return 0;

		case ( 'l' ):
			strLayer = optarg;
			break;

		case ( 'c' ):
			strCollisionlay = optarg;
			break;

		case ( 'r' ):
			iOffset = std::stoi ( optarg );
			break;

		case ( 'p' ):
			iPalette = std::stoi ( optarg );
			break;

		case ( 'i' ):
			strInPath = optarg;
			break;

		case ( 'o' ):
			strOutPath = optarg;
			break;

		default:
			break;
		}
	}

	// Check my paranoia.
	if ( strInPath.empty () )
	{
		std::cerr << "No input file specified." << std::endl;
		std::cout << g_strUsage << std::endl;
		return -1;
	}
	if ( strOutPath.empty () )
	{
		std::cerr << "No output file specified." << std::endl;
		std::cout << g_strUsage << std::endl;
		return -1;
	}
	if ( iPalette < 0 || iPalette > 15 )
	{
		std::cerr << "Invalid palette index." << std::endl;
		std::cout << g_strUsage << std::endl;
		return -1;
	}

	// Open & read input file.
	std::ifstream fin ( strInPath );
	if ( !fin.is_open () )
	{
		std::cerr << "Failed to open input file." << std::endl;
		return -1;
	}
	std::stringstream buf;
	buf << fin.rdbuf ();
	fin.close ();
	std::string strXml = buf.str ();
	buf.clear ();

	STilemapInfo info;
	memset ( &info, 0, sizeof(STilemapInfo) );
	std::string strEncData, strEncCollisionDat;

	// Parse document.
	rapidxml::xml_document<> xDoc;
	xDoc.parse<0> ( (char*)strXml.c_str () );

	// Get map node.
	auto xMap = xDoc.first_node ( "map" );
	if ( xMap != nullptr )
	{
		// Read map attribs.
		rapidxml::xml_attribute<>* xAttrib = nullptr;
		if ( ( xAttrib = xMap->first_attribute ( "width" ) ) != nullptr )
		{
			info.width = std::stoi ( xAttrib->value () );
		}
		if ( ( xAttrib = xMap->first_attribute ( "height" ) ) != nullptr )
		{
			info.height = std::stoi ( xAttrib->value () );
		}

		// Get layer node.
		rapidxml::xml_node<>* xLayer = nullptr;
		if ( strLayer.empty () )
		{
			xLayer = xMap->first_node ( "layer" );
		}
		else
		{
			// Find specified layer.
			for ( auto xNode = xMap->first_node (); xNode != nullptr; xNode = xNode->next_sibling () )
			{
				// We only want layers.
				if ( strcmp ( xNode->name (), "layer" ) == 0 )
				{
					// Use this layer if it matches our specified name.
					auto xName = xNode->first_attribute ( "name" );
					if ( xName != nullptr )
					{
						if ( strcmp ( xName->value (), strLayer.c_str () ) == 0 )
						{
							xLayer = xNode;
							break;
						}
					}
				}
			}
		}

		// Find collision layer.
		rapidxml::xml_node<>* xCollisionLay = nullptr;
		if ( !strCollisionlay.empty () )
		{
			for ( auto xNode = xMap->first_node (); xNode != nullptr; xNode = xNode->next_sibling () )
			{
				if ( strcmp ( xNode->name (), "layer" ) == 0 )
				{
					// Use this layer if it matches our specified name.
					auto xName = xNode->first_attribute ( "name" );
					if ( xName != nullptr )
					{
						if ( strcmp ( xName->value (), strCollisionlay.c_str () ) == 0 )
						{
							xCollisionLay = xNode;
							break;
						}
					}
				}
			}
		}

		// Read data from layer node.
		if ( xLayer != nullptr )
		{
			auto xData = xLayer->first_node ( "data" );
			if ( xData != nullptr )
			{
				// Get encoded data.
				strEncData = xData->value ();
			}
		}

		// Read data from collision layer.
		if ( xCollisionLay != nullptr )
		{
			auto xData = xCollisionLay->first_node ( "data" );
			if ( xData != nullptr )
			{
				// Get encoded data.
				strEncCollisionDat = xData->value ();
			}
		}
	}

	// Check what we (don't) have.
	if ( info.width == 0 || info.height == 0 || strEncData.empty () )
	{
		std::cerr << "Parse error.";
		return -1;
	}

	// Decode and decompress layer data.
	std::vector<uint8_t> vucLayerDat;
	if ( !DecodeMapData ( strEncData, info.width, info.height, &vucLayerDat ) )
	{
		std::cerr << "Decompression error.";
		return -1;
	}

	// Convert to GBA-friendly charmap data.
	uint16_t* pRead = (uint16_t*)vucLayerDat.data ();
	std::vector<uint16_t> vucCharDat;
	vucCharDat.reserve ( info.width * info.height );
	for ( size_t i = 0; i < size_t(info.width * info.height * 2); ++i )
	{
		uint16_t usTile = std::max<int> ( (*pRead++) + iOffset, 0 );

		bool bFlipH = ( 0x8000 & *pRead ) ? true : false;
		bool bFlipV = ( 0x4000 & *pRead++ ) ? true : false;

		uint8_t ucFlags = 0x0;
		ucFlags |= ( bFlipH ) ? 0x4 : 0x0;
		ucFlags |= ( bFlipV ) ? 0x8 : 0x0;
		ucFlags |= iPalette << 4;

		vucCharDat.push_back ( usTile | ucFlags << 8 );
	}
	vucLayerDat.clear ();

	// Save out charmap.
	std::ofstream fout ( strOutPath, std::ios::binary );
	if ( !fout.is_open () )
	{
		std::cerr << "Failed to create output file.";
		return -1;
	}
	fout.write ( (const char*)vucCharDat.data (), vucCharDat.size () );
	fout.close ();

	// Decode & convert collision map.
	std::vector<uint8_t> vucCollisionDat;
	if ( !strEncCollisionDat.empty () )
	{
		std::vector<uint8_t> vucLayerDat;
		if ( DecodeMapData ( strEncCollisionDat, info.width, info.height, &vucLayerDat ) )
		{
			vucCollisionDat.reserve ( info.width * info.height );
			uint8_t* pRead = vucLayerDat.data ();
			for ( size_t i = 0; i < info.width * info.height; ++i )
			{
				uint8_t ucTile = *pRead;
				pRead += 4;
				vucCollisionDat.push_back ( ucTile );
			}
		}
	}

	// Save it out or something like that.
	if ( !vucCollisionDat.empty () )
	{
		// Try to nicely append "_collision" to the output name.
		std::string strPath;
		size_t extPos = strOutPath.find_last_of ( '.' );
		if ( extPos != std::string::npos )
		{
			strPath = strOutPath.insert ( extPos, "_collision" );
		}
		else
		{
			strPath = strOutPath + "_collision";
		}

		// Save it out.
		fout.open ( strPath, std::ios::binary );
		if ( fout.is_open () )
		{
			fout.write ( (const char*)vucCollisionDat.data (), vucCollisionDat.size () );
			fout.close ();
		}
	}

	return 0;
}
