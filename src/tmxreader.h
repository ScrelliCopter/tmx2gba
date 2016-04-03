#ifndef __TMXREADER_H__
#define __TMXREADER_H__

#include <istream>
#include <vector>
#include <string>
#include <cstdint>

class CTmxTileset;
class CTmxLayer;
class CTmxObject;
namespace rapidxml { template<class Ch = char> class xml_node; }

class CTmxReader
{
public:
	CTmxReader ();
	~CTmxReader ();

	void Open ( std::istream& a_in );

	int GetWidth () const;
	int GetHeight () const;

	const CTmxLayer* GetLayer ( int a_id ) const;
	const CTmxLayer* GetLayer ( std::string a_strName ) const;
	int GetLayerCount () const;

	const CTmxObject* GetObject ( int a_id ) const;
	int GetObjectCount () const;

	uint32_t LidFromGid ( uint32_t a_gid );

private:
	bool DecodeMap ( uint32_t* a_out, size_t a_outSize, const std::string& a_strIn );
	void ReadTileset ( rapidxml::xml_node<>* a_xNode );
	void ReadLayer ( rapidxml::xml_node<>* a_xNode );
	void ReadObjects ( rapidxml::xml_node<>* a_xNode );

	int m_width, m_height;
	std::vector<CTmxTileset*>	m_tileset;
	std::vector<CTmxLayer*>		m_layers;
	std::vector<CTmxObject*>	m_objects;
	std::vector<uint32_t>		m_gidTable;

};

#endif//__TMXREADER_H__
