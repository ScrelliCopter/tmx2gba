add_executable(tmx2gba
	argparse.hpp argparse.cpp
	tmxlayer.hpp
	tmxobject.hpp
	tmxreader.hpp tmxreader.cpp
	tmxtileset.hpp
	swriter.hpp
	headerwriter.hpp headerwriter.cpp
	tmx2gba.cpp)

set_target_properties(tmx2gba PROPERTIES
	# C++20 & C99
	CXX_STANDARD 20
	C_STANDARD 99)

# Enable strong warnings
target_compile_options(tmx2gba PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/Wall>
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -pedantic>
	$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Weverything -Wno-c++98-compat>)

target_link_libraries(tmx2gba
	External::base64
	External::miniz
	External::rapidxml)

if (TMX2GBA_DKP_INSTALL)
	if (DEFINED ENV{DEVKITPRO})
		set(TMX2GBA_INSTALL_DESTINATION "$ENV{DEVKITPRO}/tools/bin")
	else()
		set(TMX2GBA_INSTALL_DESTINATION /opt/devkitpro/tools/bin)
	endif()
else()
	set(TMX2GBA_INSTALL_DESTINATION bin)
endif()
install(TARGETS tmx2gba RUNTIME DESTINATION "${TMX2GBA_INSTALL_DESTINATION}")
