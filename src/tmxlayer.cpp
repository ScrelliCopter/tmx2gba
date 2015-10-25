/* tmxlayer.cpp

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

#include "tmxlayer.h"


CTmxLayer::CTmxLayer () :
	m_iWidth ( 0 ),
	m_iHeight ( 0 ),
	m_strName ( "" ),
	m_pTileDat ( nullptr )
{

}

CTmxLayer::CTmxLayer ( int a_iWidth, int a_iHeight, const char* a_szName, uint8_t* a_pTileDat ) :
	m_iWidth ( a_iWidth ),
	m_iHeight ( a_iHeight ),
	m_strName ( a_szName ),
	m_pTileDat ( a_pTileDat )
{

}

CTmxLayer::~CTmxLayer ()
{
	delete[] m_pTileDat;
}


const std::string& CTmxLayer::GetName () const
{
	return m_strName;
}

int CTmxLayer::GetWidth () const
{
	return m_iWidth;
}

int CTmxLayer::GetHeight () const
{
	return m_iHeight;
}

const uint8_t* CTmxLayer::GetData () const
{
	return m_pTileDat;
}
