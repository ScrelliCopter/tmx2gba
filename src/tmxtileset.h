#ifndef __TMXTILESET_H__
#define __TMXTILESET_H__

#include <string>
#include <cstdint>

class CTmxTileset
{
public:
	CTmxTileset ();
	CTmxTileset ( const char* a_szName, const char* a_szSource, uint32_t a_uiFirstGid );
	~CTmxTileset ();

	const std::string& GetName () const;
	const std::string& GetSource () const;
	uint32_t GetFirstGid () const;

private:
	std::string m_strName;
	std::string m_strSource;
	uint32_t m_uiFirstGid;

};

#endif//__TMXTILESET_H__
