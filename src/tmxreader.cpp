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
#include "tmxlayer.h"
#include <cstdint>
#include <sstream>
#include <rapidxml/rapidxml.hpp>
#include <base64.h>
#include <miniz.h>


CTmxReader::CTmxReader ()
{

}

CTmxReader::~CTmxReader ()
{
	// Delete old layers.
	for ( auto pLay : m_vpLayers )
	{
		delete pLay;
	}
	m_vpLayers.clear ();
}


bool DecodeMap ( uint8_t* a_szOut, size_t a_outSize, const std::string& a_strIn )
{
	// Decode base64 string.
	size_t cutTheCrap = a_strIn.find_first_not_of ( " \t\n\r" );
	std::string strDec = base64_decode ( a_strIn.substr ( cutTheCrap ) );

	// Decompress compressed data.
	mz_ulong uiDstSize = a_outSize;
	int iRes = uncompress (
			a_szOut, &uiDstSize,
			(const uint8_t*)strDec.data (), strDec.size ()
		);
	strDec.clear ();
	if ( iRes < 0 )
	{
		return false;
	}

	return true;
}

void CTmxReader::Open ( std::istream& a_in )
{
	// Delete old layers.
	for ( auto pLay : m_vpLayers )
	{
		delete pLay;
	}
	m_vpLayers.clear ();

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
		m_iWidth = std::stoi ( xAttrib->value () );
	}
	if ( ( xAttrib = xMap->first_attribute ( "height" ) ) != nullptr )
	{
		m_iHeight = std::stoi ( xAttrib->value () );
	}

	// Read layer nodes.
	for ( auto xNode = xMap->first_node (); xNode != nullptr; xNode = xNode->next_sibling () )
	{
		// Make sure it's a layer.
		if ( strcmp ( xNode->name (), "layer" ) != 0 )
		{
			continue;
		}

		rapidxml::xml_attribute<>* xAttrib;
		const char*	szName		= nullptr;
		int			iWidth		= 0;
		int			iHeight		= 0;
		uint8_t*	pTileDat	= nullptr;

		// Read name.
		xAttrib = xNode->first_attribute ( "name" );
		if ( xAttrib != nullptr )
		{
			szName = xAttrib->value ();
		}

		// Read width.
		xAttrib = xNode->first_attribute ( "width" );
		if ( xAttrib != nullptr )
		{
			iWidth = std::stoi ( xAttrib->value () );
		}

		// Read height.
		xAttrib = xNode->first_attribute ( "height" );
		if ( xAttrib != nullptr )
		{
			iHeight = std::stoi ( xAttrib->value () );
		}

		// Read tile data.
		auto xData = xNode->first_node ( "data" );
		if ( xData != nullptr )
		{
			// TODO: don't assume base64 & zlib.
			pTileDat = new uint8_t[iWidth * iHeight * 4];
			if ( !DecodeMap ( pTileDat, iWidth * iHeight * 4, std::string ( xData->value () ) ) )
			{
				pTileDat = nullptr;
			}
		}

		m_vpLayers.push_back ( new CTmxLayer ( iWidth, iHeight, szName, pTileDat ) );
	}
}


int CTmxReader::GetWidth () const
{
	return m_iWidth;
}

int CTmxReader::GetHeight () const
{
	return m_iHeight;
}


const CTmxLayer* CTmxReader::GetLayer ( int a_iId ) const
{
	return m_vpLayers[a_iId];
}

const CTmxLayer* CTmxReader::GetLayer ( std::string a_strName ) const
{
	for ( auto pLay : m_vpLayers )
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
	return m_vpLayers.size ();
}
