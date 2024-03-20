cmake_minimum_required(VERSION "3.15" FATAL_ERROR)
project(tmx2gba VERSION "0.3")

# Options
option(TMX2GBA_DKP_INSTALL "Install into DEVKITPRO prefix" OFF)
option(ASAN "Enable address sanitiser" OFF)

if (ASAN)
	add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
	add_link_options(-fsanitize=address -shared-libasan)
endif()

# Libraries
add_subdirectory(ext/base64)
add_subdirectory(ext/miniz)
add_subdirectory(ext/rapidxml)

# Main tmx2gba sources
add_subdirectory(src)

if (MSVC)
	# Default to tmx2gba as startup project when generating Solutions
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT tmx2gba)
endif()
