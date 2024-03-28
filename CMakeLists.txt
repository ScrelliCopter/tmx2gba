cmake_minimum_required(VERSION "3.15" FATAL_ERROR)
project(tmx2gba
	VERSION      "0.7"
	DESCRIPTION  "Simple CLI utility for converting Tiled (.tmx) maps to GBA-friendly charmaps."
	HOMEPAGE_URL "https://github.com/ScrelliCopter/tmx2gba")

# Options
option(USE_ZLIB            "Use zlib instead of bundled miniz" "${UNIX}")
option(USE_BUNDLED_PUGIXML "Use bundled PUGIXML" ON)
option(USE_BUNDLED_ZSTD    "Use bundled libzstd" ON)
option(USE_BUNDLED_TMXLITE "Use bundled tmxlite" ON)

option(TMX2GBA_DKP_INSTALL "Install into DEVKITPRO prefix" OFF)

option(ENABLE_ASAN "Enable address sanitiser" OFF)

if (ENABLE_ASAN)
	add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
	add_link_options(-fsanitize=address -shared-libasan)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Libraries
if (USE_BUNDLED_PUGIXML)
	add_subdirectory(ext/pugixml)
else()
	find_package(pugixml REQUIRED CONFIG)
endif()

if (USE_ZLIB)
	find_package(ZLIB REQUIRED)
else()
	add_subdirectory(ext/miniz)
endif()

if (USE_BUNDLED_ZSTD)
	add_subdirectory(ext/zstd)
else()
	find_package(Zstd REQUIRED)
endif()

add_subdirectory(ext/base64)

add_subdirectory(ext/tmxlite)

# Main tmx2gba sources
add_subdirectory(src)

if (MSVC)
	# Default to tmx2gba as startup project when generating Solutions
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT tmx2gba)
endif()
