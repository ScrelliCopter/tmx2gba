#ifndef __TMXLAYER_H__
#define __TMXLAYER_H__

#include <string>
#include <cstdint>

const uint32_t FLIP_HORZ = 0x80000000;
const uint32_t FLIP_VERT = 0x40000000;
const uint32_t FLIP_DIAG = 0x20000000;
const uint32_t FLIP_MASK = 0xE0000000;

class CTmxLayer
{
public:
	CTmxLayer ();
	CTmxLayer ( int a_iWidth, int a_iHeight, const char* a_szName, uint32_t* a_pTileDat );
	~CTmxLayer ();

	const			std::string& GetName () const;
	int				GetWidth	() const;
	int				GetHeight	() const;
	const uint32_t*	GetData		() const;

private:
	std::string m_strName;
	int m_iWidth, m_iHeight;
	uint32_t* m_pTileDat;

};

#endif//__TMXLAYER_H__
