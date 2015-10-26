#ifndef __TMXREADER_H__
#define __TMXREADER_H__

#include <istream>
#include <vector>
#include <string>
#include <cstdint>

class CTmxTileset;
class CTmxLayer;
namespace rapidxml { template<class Ch = char> class xml_node; }

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

	uint32_t LidFromGid ( uint32_t a_uiGid );

private:
	bool DecodeMap ( uint32_t* a_pOut, size_t a_outSize, const std::string& a_strIn );
	void ReadTileset ( rapidxml::xml_node<>* a_xNode );
	void ReadLayer ( rapidxml::xml_node<>* a_xNode );

	int m_iWidth, m_iHeight;
	std::vector<CTmxTileset*>	m_vpTileset;
	std::vector<CTmxLayer*>		m_vpLayers;
	std::vector<uint32_t>		m_vuiGidTable;

};

#endif//__TMXREADER_H__
