add_executable(tmx2gba 
	tmx2gba.cpp
	tmxlayer.cpp   tmxlayer.h
	tmxobject.cpp  tmxobject.h
	tmxreader.cpp  tmxreader.h
	tmxtileset.cpp tmxtileset.h)
target_link_libraries(tmx2gba
	External::base64
	External::miniz
	External::rapidxml
	External::ultragetopt)
