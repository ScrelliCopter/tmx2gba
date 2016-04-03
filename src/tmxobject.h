#ifndef __TMXOBJECT_H__
#define __TMXOBJECT_H__

#include <string>

class CTmxObject
{
public:
	CTmxObject ();
	CTmxObject ( const std::string& a_name, float a_x, float a_y );
	~CTmxObject ();

	const std::string& GetName () const;
	void GetPos ( float* a_outX, float* a_outY ) const;

private:
	std::string m_name;
	float m_x, m_y;
};

#endif//__TMXOBJECT_H__
