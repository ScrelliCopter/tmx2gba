#ifndef __TMXLAYER_H__
#define __TMXLAYER_H__

#include <string>
#include <cstdint>

class CTmxLayer
{
public:
	CTmxLayer ();
	CTmxLayer ( int a_iWidth, int a_iHeight, const char* a_szName, uint8_t* a_pTileDat );
	~CTmxLayer ();

	const			std::string& GetName () const;
	int				GetWidth	() const;
	int				GetHeight	() const;
	const uint8_t*	GetData		() const;

private:
	std::string m_strName;
	int m_iWidth, m_iHeight;
	uint8_t* m_pTileDat;

};

#endif//__TMXLAYER_H__
