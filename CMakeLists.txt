# tmx2gba - CMakeLists.txt
# Copyright (C) 2018 Nicholas Curtis (a dinosaur)
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

cmake_minimum_required(VERSION 3.10)
project(tmx2gba)

set(CMAKE_CXX_STANDARD 11)

include_directories(inc)
set(SOURCES
	src/tmx2gba.cpp
	src/tmxlayer.cpp	src/tmxlayer.h
	src/tmxobject.cpp	src/tmxobject.h
	src/tmxreader.cpp	src/tmxreader.h
	src/tmxtileset.cpp	src/tmxtileset.h

	src/base64.cpp		inc/base64.h
	inc/miniz.h

	inc/rapidxml/rapidxml.hpp
	inc/rapidxml/rapidxml_iterators.hpp
	inc/rapidxml/rapidxml_print.hpp
	inc/rapidxml/rapidxml_utils.hpp)

if (MSVC)
	set(SOURCES ${SOURCES} src/XGetopt.cpp inc/XGetopt.h)
endif()

add_executable(tmx2gba ${SOURCES})
