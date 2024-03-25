cmake_minimum_required(VERSION "3.15" FATAL_ERROR)
project(tmx2gba
	VERSION      "0.7"
	DESCRIPTION  "Simple CLI utility for converting Tiled (.tmx) maps to GBA-friendly charmaps."
	HOMEPAGE_URL "https://github.com/ScrelliCopter/tmx2gba")

# Options
option(TMX2GBA_DKP_INSTALL "Install into DEVKITPRO prefix" OFF)
option(ASAN "Enable address sanitiser" OFF)

if (ASAN)
	add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
	add_link_options(-fsanitize=address -shared-libasan)
endif()

# Libraries
set(TMXLITE_STATIC_LIB ON)
add_subdirectory(ext/tmxlite)

# Main tmx2gba sources
add_subdirectory(src)

if (MSVC)
	# Default to tmx2gba as startup project when generating Solutions
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT tmx2gba)
endif()
