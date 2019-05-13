/* tmx2gba.cpp

  Copyright (C) 2015-2019 Nicholas Curtis (a dinosaur)

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
#include "tmxobject.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <algorithm>
#include <ultragetopt.h>


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::ifstream;
using std::ofstream;
using std::stoi;
using std::min;
using std::max;
using std::ios;

static const string g_strUsage = "Usage: tmx2gba [-h] [-f file] [-r offset] [-lyc name] [-p 0-15] [-m name;id] <-i inpath> <-o outpath>";
static const string g_strHelpShort = "Run 'tmx2gba -h' to view all available options.";
static const string g_strHelpFull = R"(
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

struct SParams
{
	bool help = false;
	string inPath, outPath;
	string layer, collisionlay, paletteLay;
	string flagFile;
	int offset = 0;
	int palette = 0;
	vector<string> objMappings;
	bool objExport = false;
};

void ParseArgs ( int argc, char** argv, SParams* params )
{
	char cOption;
	optreset = 1;
	while ( ( cOption = (char)getopt ( argc, argv, "hr:l:c:p:y:m:i:o:f:" ) ) > 0 )
	{
		switch ( cOption )
		{
		case ( 'h' ):
			params->help = true;
			return;

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
			params->offset = stoi ( optarg );
			break;

		case ( 'p' ):
			params->palette = stoi ( optarg );
			break;

		case ( 'm' ):
			params->objExport = true;
			params->objMappings.push_back ( optarg );
			break;

		case ( 'i' ):
			params->inPath = optarg;
			break;

		case ( 'o' ):
			params->outPath = optarg;
			break;

		case ( 'f' ):
			params->flagFile = optarg;
			break;

		default:
			break;
		}
	}
}

bool CheckArgs ( const SParams* params )
{
	// Check my paranoia.
	if ( params->inPath.empty () )
	{
		cerr << "No input file specified." << endl;
		cout << g_strUsage << endl << g_strHelpShort << endl;
		return false;
	}
	if ( params->outPath.empty () )
	{
		cerr << "No output file specified." << endl;
		cout << g_strUsage << endl << g_strHelpShort << endl;
		return false;
	}
	if ( params->palette < 0 || params->palette > 15 )
	{
		cerr << "Invalid palette index." << endl;
		cout << g_strUsage << endl << g_strHelpShort << endl;
		return false;
	}

	return true;
}


template <typename T>
void WriteArray ( ofstream& a_fout, const vector<T>& a_dat, int a_perCol = 16 )
{
	const int w = sizeof(T) * 2;
	int col = 0;

	string datType = "ERR";
	if ( sizeof(T) == 1 )
	{
		datType = ".byte";
	}
	else
	if ( sizeof(T) == 2 )
	{
		datType = ".hword";
	}
	else
	if ( sizeof(T) == 4 )
	{
		datType = ".word";
	}

	a_fout.setf ( ios::hex, ios::basefield );
	a_fout.setf ( ios::showbase );

	size_t i = 0;
	for ( T element : a_dat )
	{
		if ( col == 0 )
		{
			a_fout << "\t" << datType << " ";
		}

		a_fout << std::hex << (int)element;

		if ( i < a_dat.size () - 1 )
		{
			if ( ++col < a_perCol )
			{
				a_fout << ",";
			}
			else
			{
				a_fout << "" << endl;
				col = 0;
			}
		}

		++i;
	}
}

int main ( int argc, char** argv )
{
	SParams params;
	ParseArgs ( argc, argv, &params );

	if ( params.help )
	{
		cout << g_strUsage << endl << g_strHelpFull << endl;
		return 0;
	}

	if ( params.flagFile.length () != 0 )
	{
		ifstream paramFile ( params.flagFile );
		if ( !paramFile.is_open () )
		{
			cerr << "Failed to open param file." << endl;
			return -1;
		}
		
		vector<string> fileArgTokens;
		fileArgTokens.push_back ( "auu~~" );
		bool carry = false;
		string token;
		while ( !paramFile.eof () )
		{
			if ( carry )
			{
				string tmp;
				paramFile >> tmp;
				token += " ";
				token += tmp;
			}
			else
			{
				token.clear ();
				paramFile >> token;
			}

			if ( token == "" )
			{
				continue;
			}

			bool qFr = token[0] == '"';
			bool qBk = token[token.length () - 1] == '"';
			if ( qFr && qBk )
			{
				fileArgTokens.push_back ( token.substr ( 1, token.length () - 2 ) );
			}
			else
			if ( qFr )
			{
				fileArgTokens.push_back ( token.substr ( 1, token.length () - 1 ) );
				carry = true;
			}
			else
			if ( qBk )
			{
				fileArgTokens.push_back ( token.substr ( 0, token.length () - 1 ) );
				carry = false;
			}
			else
			{
				fileArgTokens.push_back ( token );
			}
		}

		vector<const char*> fileArgs;
		fileArgs.reserve ( fileArgTokens.size () );
		for ( auto& token : fileArgTokens )
		{
			fileArgs.push_back ( token.c_str () );
		}
		fileArgs.push_back(nullptr);

		ParseArgs ( fileArgs.size () - 1, (char**)fileArgs.data (), &params );
	}

	if ( !CheckArgs ( &params ) )
	{
		return -1;
	}

	// Object mappings.
	map<string, uint32_t> objMapping;
	if ( params.objExport )
	{
		for ( auto token : params.objMappings )
		{
			int splitter = token.find_last_of ( ';' );
			if ( splitter == -1 )
			{
				cerr << "Malformed mapping (missing a splitter)." << endl;
				return -1;
			}

			try
			{
				string name = token.substr ( 0, splitter );
				int id = stoi ( token.substr ( splitter + 1 ) );

				objMapping[name] = id;
			}
			catch ( std::exception )
			{
				cerr << "Malformed mapping, make sure id is numeric." << endl;
			}
		}
	}

	// Open & read input file.
	CTmxReader tmx;
	ifstream fin ( params.inPath );
	if ( !fin.is_open () )
	{
		cerr << "Failed to open input file." << endl;
		return -1;
	}
	tmx.Open ( fin );

	// Get layers.
	if ( tmx.GetLayerCount () == 0 )
	{
		cerr << "No layers found." << endl; 
		return -1;
	}
	const CTmxLayer* pLayerGfx = params.layer.empty () ? tmx.GetLayer ( 0 ) : tmx.GetLayer ( params.layer );
	const CTmxLayer* pLayerCls = params.collisionlay.empty () ? nullptr : tmx.GetLayer ( params.collisionlay );
	const CTmxLayer* pLayerPal = params.paletteLay.empty () ? nullptr : tmx.GetLayer ( params.paletteLay );

	if ( pLayerGfx == nullptr )
	{
		cerr << "Input layer not found." << endl;
		return -1;
	}

	// Open output files.
	ofstream foutS ( params.outPath + ".s" );
	ofstream foutH ( params.outPath + ".h" );
	if ( !foutS.is_open () || !foutH.is_open () )
	{
		cerr << "Failed to create output file(s).";
		return -1;
	}

	int slashPos = max ( (int)params.outPath.find_last_of ( '/' ), (int)params.outPath.find_last_of ( '\\' ) );
	string name = params.outPath;
	if ( slashPos != -1 )
	{
		name = name.substr ( slashPos + 1 );
	}

	// Write header guards.
	string guard = "__TMX2GBA_" + name + "__";
	for ( auto& c: guard ) c = (char)toupper ( (int)c );
	foutH << "#ifndef " << guard << endl;
	foutH << "#define " << guard << endl;
	foutH << endl;
	foutH << "#define " << name << "Width " << tmx.GetWidth () << endl;
	foutH << "#define " << name << "Height " << tmx.GetHeight () << endl;
	foutH << endl;

	// Convert to GBA-friendly charmap data.
	const uint32_t* pRead		= pLayerGfx->GetData ();
	const uint32_t* pPalRead	= pLayerPal == nullptr ? nullptr : pLayerPal->GetData ();
	vector<uint16_t> vucCharDat;
	vucCharDat.reserve ( pLayerGfx->GetWidth () * pLayerGfx->GetHeight () );
	for ( size_t i = 0; i < size_t(pLayerGfx->GetWidth () * pLayerGfx->GetHeight ()); ++i )
	{
		uint32_t uiRead = (*pRead++);

		uint16_t usTile	= (uint16_t)max<int32_t> ( 0, tmx.LidFromGid ( uiRead & ~FLIP_MASK ) + params.offset );
		uint8_t ucFlags	= 0x0;

		// Get flipped!
		ucFlags |= ( uiRead & FLIP_HORZ ) ? 0x4 : 0x0;
		ucFlags |= ( uiRead & FLIP_VERT ) ? 0x8 : 0x0;

		// Determine palette ID.
		uint32_t uiIndex = 0;
		if ( pPalRead != nullptr )
		{
			uiIndex = tmx.LidFromGid ( (*pPalRead++) & ~FLIP_MASK );
		}
		if ( uiIndex == 0 )
		{
			uiIndex = params.palette + 1;
		}
		ucFlags |= (uint8_t)(uiIndex - 1) << 4;

		vucCharDat.push_back ( usTile | ( uint16_t(ucFlags) << 8 ) );
	}

	// Save out charmap.
	foutH << "#define " << name << "TilesLen " << vucCharDat.size () * 2 << endl;
	foutH << "extern const unsigned short " << name << "Tiles[" << vucCharDat.size () << "];" << endl;
	foutH << endl;

	foutS << "\t.section .rodata" << endl;
	foutS << "\t.align 2" << endl;
	foutS << "\t.global " << name << "Tiles" << endl;
	foutS << "\t.hidden " << name << "Tiles" << endl;
	foutS << name << "Tiles" << ":" << endl;
	WriteArray<uint16_t> ( foutS, vucCharDat );
	foutS << endl;

	// Convert collision map & save it out.
	if ( pLayerCls != nullptr )
	{
		vector<uint8_t> vucCollisionDat;
		vucCollisionDat.reserve ( pLayerCls->GetWidth () * pLayerCls->GetHeight () );

		pRead = pLayerCls->GetData ();
		for ( int i = 0; i < pLayerCls->GetWidth () * pLayerCls->GetHeight (); ++i )
		{
			uint8_t ucTile = (uint8_t)tmx.LidFromGid ( (*pRead++) & ~FLIP_MASK );
			vucCollisionDat.push_back ( ucTile );
		}

		// Try to nicely append "_collision" to the output name.
		string strPath;
		size_t extPos = params.outPath.find_last_of ( '.' );
		if ( extPos != string::npos )
		{
			strPath = params.outPath.insert ( extPos, "_collision" );
		}
		else
		{
			strPath = params.outPath + "_collision";
		}

		// Save it out.
		foutH << "#define " << name << "CollisionLen " << vucCollisionDat.size () << endl;
		foutH << "extern const unsigned char " << name << "Collision[" << vucCollisionDat.size () << "];" << endl;
		foutH << endl;

		foutS << endl;
		foutS << "\t.section .rodata" << endl;
		foutS << "\t.align 2" << endl;
		foutS << "\t.global " << name << "Collision" << endl;
		foutS << "\t.hidden " << name << "Collision" << endl;
		foutS << name << "Collision" << ":" << endl;
		WriteArray<uint8_t> ( foutS, vucCollisionDat );
		foutS << endl;
	}

	if ( params.objExport )
	{
		vector<uint32_t> objDat;
		for ( int i = 0; i < tmx.GetObjectCount (); ++i )
		{
			auto obj = tmx.GetObject ( i );
			auto it = objMapping.find ( obj->GetName () );
			if ( it == objMapping.end () )
			{
				continue;
			}

			float x, y;
			obj->GetPos ( &x, &y );
			objDat.push_back ( it->second );
			objDat.push_back ( (int)( x * 256.0f ) );
			objDat.push_back ( (int)( y * 256.0f ) );
		}

		// Save it out.
		foutH << "#define " << name << "ObjCount " << objDat.size () / 3 << endl;
		foutH << "#define " << name << "ObjdatLen " << objDat.size () * sizeof(int) << endl;
		foutH << "extern const unsigned int " << name << "Objdat[" << objDat.size () << "];" << endl;
		foutH << endl;

		foutS << endl;
		foutS << "\t.section .rodata" << endl;
		foutS << "\t.align 2" << endl;
		foutS << "\t.global " << name << "Objdat" << endl;
		foutS << "\t.hidden " << name << "Objdat" << endl;
		foutS << name << "Objdat" << ":" << endl;
		WriteArray<uint32_t> ( foutS, objDat );
		foutS << endl;
	}

	foutH << "#endif//" << guard << endl;

	foutH.close ();
	foutS.close ();

	return 0;
}
