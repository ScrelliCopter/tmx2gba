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

#include "tmxreader.h"
#include "tmxlayer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
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


struct SParams
{
	std::string inPath, outPath;
	std::string layer, collisionlay, paletteLay;
	int offset = 0;
	int palette = 0;
};

bool ParseArgs ( int argc, char** argv, SParams* params )
{
	char cOption;
	while ( ( cOption = getopt ( argc, argv, "hr:l:c:p:y:i:o:" ) ) > 0 )
	{
		switch ( cOption )
		{
		case ( 'h' ):
			std::cout << g_strFullHelp << std::endl;
			return 0;

		case ( 'l' ):
			params->layer = optarg;
			break;

		case ( 'c' ):
			params->collisionlay = optarg;
			break;

		case ( 'y' ):
			params->paletteLay = optarg;
			break;

		case ( 'r' ):
			params->offset = std::stoi ( optarg );
			break;

		case ( 'p' ):
			params->palette = std::stoi ( optarg );
			break;

		case ( 'i' ):
			params->inPath = optarg;
			break;

		case ( 'o' ):
			params->outPath = optarg;
			break;

		default:
			break;
		}
	}

	// Check my paranoia.
	if ( params->inPath.empty () )
	{
		std::cerr << "No input file specified." << std::endl;
		std::cout << g_strUsage << std::endl;
		return false;
	}
	if ( params->outPath.empty () )
	{
		std::cerr << "No output file specified." << std::endl;
		std::cout << g_strUsage << std::endl;
		return false;
	}
	if ( params->palette < 0 || params->palette > 15 )
	{
		std::cerr << "Invalid palette index." << std::endl;
		std::cout << g_strUsage << std::endl;
		return false;
	}

	return true;
}


int main ( int argc, char** argv )
{
	SParams params;
	if ( !ParseArgs ( argc, argv, &params ) )
	{
		return -1;
	}

	// Open & read input file.
	CTmxReader tmx;
	std::ifstream fin ( params.inPath );
	if ( !fin.is_open () )
	{
		std::cerr << "Failed to open input file." << std::endl;
		return -1;
	}
	tmx.Open ( fin );

	// Get layers.
	if ( tmx.GetLayerCount () == 0 )
	{
		std::cerr << "No layers found." << std::endl; 
		return -1;
	}
	const CTmxLayer* pLayerGfx = params.inPath.empty () ? tmx.GetLayer ( 0 ) : tmx.GetLayer ( params.layer );
	const CTmxLayer* pLayerCls = params.inPath.empty () ? nullptr : tmx.GetLayer ( params.collisionlay );
	const CTmxLayer* pLayerPal = params.inPath.empty () ? nullptr : tmx.GetLayer ( params.paletteLay );

	if ( pLayerGfx == nullptr )
	{
		std::cerr << "Input layer not found." << std::endl;
		return -1;
	}

	// Convert to GBA-friendly charmap data.
	const uint16_t* pRead		= (const uint16_t*)pLayerGfx->GetData ();
	//const uint16_t* pPalRead	= (const uint16_t*)pLayerPal->GetData ();
	std::vector<uint16_t> vucCharDat;
	vucCharDat.reserve ( pLayerGfx->GetWidth () * pLayerGfx->GetHeight () );
	for ( size_t i = 0; i < size_t(pLayerGfx->GetWidth () * pLayerGfx->GetHeight () * 2); ++i )
	{
		uint16_t usTile = std::max<uint16_t> ( (*pRead++) + (uint16_t)params.offset, 0 );

		bool bFlipH = ( 0x8000 & *pRead ) ? true : false;
		bool bFlipV = ( 0x4000 & *pRead++ ) ? true : false;

		uint8_t ucFlags = 0x0;
		ucFlags |= ( bFlipH ) ? 0x4 : 0x0;
		ucFlags |= ( bFlipV ) ? 0x8 : 0x0;
		ucFlags |= params.palette << 4;

		vucCharDat.push_back ( usTile | ucFlags << 8 );
	}

	// Save out charmap.
	std::ofstream fout ( params.outPath, std::ios::binary );
	if ( !fout.is_open () )
	{
		std::cerr << "Failed to create output file.";
		return -1;
	}
	fout.write ( (const char*)vucCharDat.data (), vucCharDat.size () );
	fout.close ();

	// Convert collision map & save it out.
	if ( pLayerCls != nullptr )
	{
		std::vector<uint8_t> vucCollisionDat;
		vucCollisionDat.reserve ( pLayerCls->GetWidth () * pLayerCls->GetHeight () );

		const uint8_t* pRead = (const uint8_t*)pLayerCls->GetData ();
		for ( size_t i = 0; i < pLayerCls->GetWidth () * pLayerCls->GetHeight (); ++i )
		{
			uint8_t ucTile = *pRead;
			pRead += 4;
			vucCollisionDat.push_back ( ucTile );
		}

		// Try to nicely append "_collision" to the output name.
		std::string strPath;
		size_t extPos = params.outPath.find_last_of ( '.' );
		if ( extPos != std::string::npos )
		{
			strPath = params.outPath.insert ( extPos, "_collision" );
		}
		else
		{
			strPath = params.outPath + "_collision";
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
