/* tmxreader.cpp

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
#include "tmxtileset.h"
#include "tmxobject.h"
#include "tmxlayer.h"
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <rapidxml/rapidxml.hpp>
#include <base64.h>
#include <miniz.h>


CTmxReader::CTmxReader ()
{

}

CTmxReader::~CTmxReader ()
{
	// Delete old tilesets.
	for ( auto pTileset : m_tileset )
	{
		delete pTileset;
	}
	m_tileset.clear ();

	// Delete old layers.
	for ( auto pLay : m_layers )
	{
		delete pLay;
	}
	m_layers.clear ();
}


bool CTmxReader::DecodeMap ( uint32_t* a_pOut, size_t a_outSize, const std::string& a_strIn )
{
	// Decode base64 string.
	size_t cutTheCrap = a_strIn.find_first_not_of ( " \t\n\r" );
	std::string strDec = base64_decode ( a_strIn.substr ( cutTheCrap ) );

	// Decompress compressed data.
	mz_ulong uiDstSize = a_outSize;
	int iRes = uncompress (
			(unsigned char*)a_pOut, &uiDstSize,
			(const unsigned char*)strDec.data (), strDec.size ()
		);
	strDec.clear ();
	if ( iRes < 0 )
	{
		return false;
	}

	return true;
}

void CTmxReader::ReadTileset ( rapidxml::xml_node<>* a_xNode )
{
	rapidxml::xml_attribute<>* xAttrib;

	const char*	szName		= "";
	const char*	szSource	= "";
	uint32_t	uiFirstGid	= 0;

	// Read name.
	xAttrib = a_xNode->first_attribute ( "name" );
	if ( xAttrib != nullptr )
	{
		szName = xAttrib->value ();
	}

	// Read source.
	xAttrib = a_xNode->first_attribute ( "source" );
	if ( xAttrib != nullptr )
	{
		szSource = xAttrib->value ();
	}

	// Read first global ID.
	xAttrib = a_xNode->first_attribute ( "firstgid" );
	if ( xAttrib != nullptr )
	{
		uiFirstGid = std::stoul ( xAttrib->value () );
	}

	m_tileset.push_back ( new CTmxTileset ( szName, szSource, uiFirstGid ) );
}

void CTmxReader::ReadLayer ( rapidxml::xml_node<>* a_xNode )
{
	rapidxml::xml_attribute<>* xAttrib;
	const char*	szName		= "";
	int			iWidth		= 0;
	int			iHeight		= 0;
	uint32_t*	pTileDat	= nullptr;

	// Read name.
	xAttrib = a_xNode->first_attribute ( "name" );
	if ( xAttrib != nullptr )
	{
		szName = xAttrib->value ();
	}

	// Read width.
	xAttrib = a_xNode->first_attribute ( "width" );
	if ( xAttrib != nullptr )
	{
		iWidth = std::stoi ( xAttrib->value () );
	}

	// Read height.
	xAttrib = a_xNode->first_attribute ( "height" );
	if ( xAttrib != nullptr )
	{
		iHeight = std::stoi ( xAttrib->value () );
	}

	// Read tile data.
	auto xData = a_xNode->first_node ( "data" );
	if ( xData != nullptr )
	{
		// TODO: don't assume base64 & zlib.
		pTileDat = new uint32_t[iWidth * iHeight];
		if ( !DecodeMap ( pTileDat, iWidth * iHeight * sizeof(uint32_t), std::string ( xData->value () ) ) )
		{
			delete[] pTileDat;
			pTileDat = nullptr;
		}
	}

	m_layers.push_back ( new CTmxLayer ( iWidth, iHeight, szName, pTileDat ) );
}

void CTmxReader::ReadObjects ( rapidxml::xml_node<>* a_xNode )
{
	for ( auto xNode = a_xNode->first_node (); xNode != nullptr; xNode = xNode->next_sibling () )
	{
		if ( strcmp ( xNode->name (), "object" ) != 0 )
		{
			continue;
		}

		rapidxml::xml_attribute<>* xAttrib;
		const char*	name	= "";
		float		x		= 0.0f;
		float		y		= 0.0f;

		// Read name.
		xAttrib = xNode->first_attribute ( "name" );
		if ( xAttrib != nullptr )
		{
			name = xAttrib->value ();
		}

		// Read X pos.
		xAttrib = xNode->first_attribute ( "x" );
		if ( xAttrib != nullptr )
		{
			x = std::stof ( xAttrib->value () );
		}

		// Read Y pos.
		xAttrib = xNode->first_attribute ( "y" );
		if ( xAttrib != nullptr )
		{
			y = std::stof ( xAttrib->value () );
		}

		m_objects.push_back ( new CTmxObject ( name, x, y ) );
	}
}

void CTmxReader::Open ( std::istream& a_in )
{
	// Delete old tilesets.
	for ( auto pTileset : m_tileset )
	{
		delete pTileset;
	}
	m_tileset.clear ();

	// Delete old layers.
	for ( auto pLay : m_layers )
	{
		delete pLay;
	}
	m_layers.clear ();

	m_gidTable.clear ();

	// Read string into a buffer.
	std::stringstream buf;
	buf << a_in.rdbuf ();
	std::string strXml = buf.str ();
	buf.clear ();

	// Parse document.
	rapidxml::xml_document<> xDoc;
	xDoc.parse<0> ( (char*)strXml.c_str () );

	// Get map node.
	auto xMap = xDoc.first_node ( "map" );
	if ( xMap == nullptr )
	{
		return;
	}

	// Read map attribs.
	rapidxml::xml_attribute<>* xAttrib = nullptr;
	if ( ( xAttrib = xMap->first_attribute ( "width" ) ) != nullptr )
	{
		m_width = std::stoi ( xAttrib->value () );
	}
	if ( ( xAttrib = xMap->first_attribute ( "height" ) ) != nullptr )
	{
		m_height = std::stoi ( xAttrib->value () );
	}

	// Read nodes.
	for ( auto xNode = xMap->first_node (); xNode != nullptr; xNode = xNode->next_sibling () )
	{
		// Read layer nodes.
		if ( strcmp ( xNode->name (), "layer" ) == 0 )
		{
			ReadLayer ( xNode );
		}
		else
		if ( strcmp ( xNode->name (), "tileset" ) == 0 )
		{
			ReadTileset ( xNode );
		}
		else
		if ( strcmp ( xNode->name (), "objectgroup" ) == 0 )
		{
			ReadObjects ( xNode );
		}
	}

	// Generate global id table.
	for ( auto pTileset : m_tileset )
	{
		m_gidTable.push_back ( pTileset->GetFirstGid () );
	}
	std::sort ( m_gidTable.rbegin (), m_gidTable.rend () );
}


int CTmxReader::GetWidth () const
{
	return m_width;
}

int CTmxReader::GetHeight () const
{
	return m_height;
}


const CTmxLayer* CTmxReader::GetLayer ( int a_id ) const
{
	return m_layers[a_id];
}

const CTmxLayer* CTmxReader::GetLayer ( std::string a_strName ) const
{
	for ( auto pLay : m_layers )
	{
		if ( pLay->GetName ().compare ( a_strName ) == 0 )
		{
			return pLay;
		}
	}

	return nullptr;
}

int CTmxReader::GetLayerCount () const
{
	return m_layers.size ();
}


const CTmxObject* CTmxReader::GetObject ( int a_id ) const
{
	return m_objects[a_id];
}

int CTmxReader::GetObjectCount () const
{
	return m_objects.size ();
}


uint32_t CTmxReader::LidFromGid ( uint32_t a_uiGid )
{
	for ( uint32_t uiFirst : m_gidTable )
	{
		if ( uiFirst <= a_uiGid )
		{
			return a_uiGid - ( uiFirst - 1 );
		}
	}

	return a_uiGid;
}
