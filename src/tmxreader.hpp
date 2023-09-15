/* tmxreader.hpp - Copyright (C) 2015-2022 a dinosaur (zlib, see COPYING.txt) */

#ifndef TMXREADER_H
#define TMXREADER_H

#include <istream>
#include <vector>
#include <string>
#include <cstdint>
#include <rapidxml/rapidxml.hpp>

class TmxTileset;
class TmxLayer;
class TmxObject;

class TmxReader
{
public:
	TmxReader() = default;
	~TmxReader();

	void Open(std::istream& aIn);

	constexpr int GetWidth() const { return mWidth; }
	constexpr int GetHeight() const { return mHeight; }

	inline const TmxLayer* GetLayer(size_t aId) const { return mLayers.at(aId); }
	const TmxLayer* GetLayer(const std::string& aName) const;
	inline std::size_t GetLayerCount() const { return mLayers.size(); }

	inline const TmxObject* GetObject(size_t aId) const { return mObjects.at(aId); }
	inline size_t GetObjectCount() const { return mObjects.size(); }

	uint32_t LidFromGid(uint32_t aGid);

private:
	static bool DecodeMap(uint32_t* aOut, size_t aOutSize, const std::string& aBase64Dat);
	void ReadTileset(rapidxml::xml_node<>* aXNode);
	void ReadLayer(rapidxml::xml_node<>* aXNode);
	void ReadObjects(rapidxml::xml_node<>* aXNode);

	int mWidth, mHeight;
	std::vector<TmxTileset*> mTilesets;
	std::vector<TmxLayer*>   mLayers;
	std::vector<TmxObject*>  mObjects;
	std::vector<uint32_t>    mGidTable;

};

#endif//TMXREADER_H
