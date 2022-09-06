add_executable(tmx2gba 
	tmx2gba.cpp
	tmxlayer.hpp
	tmxobject.hpp
	tmxreader.hpp tmxreader.cpp
	tmxtileset.hpp)
target_link_libraries(tmx2gba
	External::base64
	External::miniz
	External::rapidxml
	External::ultragetopt)
