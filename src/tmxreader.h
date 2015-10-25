#ifndef __TMXREADER_H__
#define __TMXREADER_H__

#include <istream>
#include <vector>

class CTmxLayer;

class CTmxReader
{
public:
	CTmxReader ();
	~CTmxReader ();

	void Open ( std::istream& a_in );

	int GetWidth () const;
	int GetHeight () const;

	const CTmxLayer* GetLayer ( int a_iId ) const;
	const CTmxLayer* GetLayer ( std::string a_strName ) const;
	int GetLayerCount () const;

private:
	int m_iWidth, m_iHeight;
	std::vector<CTmxLayer*> m_vpLayers;

};

#endif//__TMXREADER_H__
