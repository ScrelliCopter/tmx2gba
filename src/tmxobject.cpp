/* tmxobject.cpp

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

#include "tmxobject.h"

CTmxObject::CTmxObject () :
	m_name ( "" ),
	m_x ( 0.0f ), m_y ( 0.0f )
{

}

CTmxObject::CTmxObject ( const std::string& a_name, float a_x, float a_y ) :
	m_name ( a_name ),
	m_x ( a_x ), m_y ( a_y )
{

}

CTmxObject::~CTmxObject ()
{

}


const std::string& CTmxObject::GetName () const
{
	return m_name;
}

void CTmxObject::GetPos ( float* a_outX, float* a_outY ) const
{
	if ( a_outX != nullptr )
	{
		*a_outX = m_x;
	}
	if ( a_outY != nullptr )
	{
		*a_outY = m_y;
	}
}

