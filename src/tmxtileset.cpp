/* tmxtileset.cpp

  Copyright (C) 2015 a dinosaur

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

#include "tmxtileset.h"

CTmxTileset::CTmxTileset () :
	m_strName		( "" ),
	m_strSource		( "" ),
	m_uiFirstGid	( 0 )
{

}

CTmxTileset::CTmxTileset ( const char* a_szName, const char* a_szSource, uint32_t a_uiFirstGid ) :
	m_strName		( a_szName ),
	m_strSource		( a_szSource ),
	m_uiFirstGid	( a_uiFirstGid )
{

}

CTmxTileset::~CTmxTileset ()
{

}


const std::string& CTmxTileset::GetName () const
{
	return m_strName;
}

const std::string& CTmxTileset::GetSource () const
{
	return m_strSource;
}

uint32_t CTmxTileset::GetFirstGid () const
{
	return m_uiFirstGid;
}
