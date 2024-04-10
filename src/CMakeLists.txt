add_executable(tmx2gba
	argparse.hpp argparse.cpp
	$<$<NOT:$<TARGET_EXISTS:ZLIB::ZLIB>>:gzip.hpp gzip.cpp>
	tmxlayer.hpp
	tmxobject.hpp
	tmxtileset.hpp
	tmxmap.hpp tmxmap.cpp
	tmxreader.hpp tmxreader.cpp
	convert.hpp convert.cpp
	headerwriter.hpp headerwriter.cpp
	swriter.hpp swriter.cpp
	tmx2gba.cpp)

configure_file(config.h.in config.h @ONLY)
target_sources(tmx2gba PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/config.h)
target_include_directories(tmx2gba PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(tmx2gba PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	$<$<BOOL:${MSVC}>:_CRT_SECURE_NO_WARNINGS>)  # Disable msvc warning

# Enable strong warnings
target_compile_options(tmx2gba PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/Wall>
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -pedantic>
	$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded>)

target_link_libraries(tmx2gba pugixml base64::base64 Zstd::Zstd
	$<$<TARGET_EXISTS:ZLIB::ZLIB>:ZLIB::ZLIB>
	$<$<TARGET_EXISTS:miniz::miniz>:miniz::miniz>)

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
