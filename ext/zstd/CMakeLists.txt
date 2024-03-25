# ################################################################
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# ################################################################

set(LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Parse version
file(READ ${LIBRARY_DIR}/zstd.h CONTENT)  # Read file content

string(REGEX MATCH ".*define ZSTD_VERSION_MAJOR *([0-9]+).*define ZSTD_VERSION_MINOR *([0-9]+).*define ZSTD_VERSION_RELEASE *([0-9]+)" VERSION_REGEX "${CONTENT}")
set(zstd_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
set(zstd_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
set(zstd_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)

enable_language(ASM)

#-----------------------------------------------------------------------------
# Add extra compilation flags
#-----------------------------------------------------------------------------
function (add_zstd_compilation_flags _target)
	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" OR MINGW)
		if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
			# clang-cl normally maps -Wall to -Weverything.
			target_compile_options(${_target} PRIVATE "/clang:-Wall")
		else()
			target_compile_options(${_target} PRIVATE -Wall)
		endif()
		target_compile_options(${_target} PRIVATE -Wextra -Wundef -Wshadow -Wcast-align -Wcast-qual)
		target_compile_options(${_target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wstrict-prototypes>)
		# Enable asserts in Debug mode
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			target_compile_options(${_target} PRIVATE DEBUGLEVEL=1)
		endif()
		# Add noexecstack flags
		target_link_options(${_target} PRIVATE -z noexecstack)  # LDFLAGS
		target_compile_options(${_target} PRIVATE -Wunused-parameter -Wa,--noexecstack)  # CFLAGS & CXXFLAGS
	elseif (MSVC)
		# UNICODE SUPPORT
		target_compile_definitions(${_target} PRIVATE _UNICODE UNICODE)
		# Enable asserts in Debug mode
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			target_compile_definitions(${_target} PRIVATE DEBUGLEVEL=1)
		endif()
	endif()
endfunction()

# Legacy support
option(ZSTD_LEGACY_SUPPORT "LEGACY SUPPORT" ON)

if (ZSTD_LEGACY_SUPPORT)
	set(ZSTD_LEGACY_LEVEL 5 CACHE STRING "")
endif()

# Multi-threading support
if (ANDROID)
	set(ZSTD_MULTITHREAD_SUPPORT_DEFAULT OFF)
else()
	set(ZSTD_MULTITHREAD_SUPPORT_DEFAULT ON)
endif()

option(ZSTD_MULTITHREAD_SUPPORT "MULTITHREADING SUPPORT" ${ZSTD_MULTITHREAD_SUPPORT_DEFAULT})

file(GLOB CommonSources ${LIBRARY_DIR}/common/*.c)
file(GLOB CompressSources ${LIBRARY_DIR}/compress/*.c)
if (MSVC)
	file(GLOB DecompressSources ${LIBRARY_DIR}/decompress/*.c)
else()
	file(GLOB DecompressSources ${LIBRARY_DIR}/decompress/*.c ${LIBRARY_DIR}/decompress/*.S)
endif()
file(GLOB DictBuilderSources ${LIBRARY_DIR}/dictBuilder/*.c)

set(Sources
	${CommonSources}
	${CompressSources}
	${DecompressSources}
	${DictBuilderSources})

file(GLOB CommonHeaders ${LIBRARY_DIR}/common/*.h)
file(GLOB CompressHeaders ${LIBRARY_DIR}/compress/*.h)
file(GLOB DecompressHeaders ${LIBRARY_DIR}/decompress/*.h)
file(GLOB DictBuilderHeaders ${LIBRARY_DIR}/dictBuilder/*.h)

set(Headers
	${LIBRARY_DIR}/zstd.h
	${CommonHeaders}
	${CompressHeaders}
	${DecompressHeaders}
	${DictBuilderHeaders})

if (ZSTD_LEGACY_SUPPORT)
	foreach (SOURCE zstd_v01.c zstd_v02.c zstd_v03.c zstd_v04.c zstd_v05.c zstd_v06.c zstd_v07.c)
		list(APPEND Sources ${LIBRARY_DIR}/legacy/${SOURCE})
	endforeach()
	foreach (HEADER zstd_legacy.h zstd_v01.h zstd_v02.h zstd_v03.h zstd_v04.h zstd_v05.h zstd_v06.h zstd_v07.h)
		list(APPEND Headers ${LIBRARY_DIR}/legacy/${HEADER})
	endforeach()
endif()

# Explicitly set the language to C for all files, including ASM files.
# Our assembly expects to be compiled by a C compiler, and is only enabled for
# __GNUC__ compatible compilers. Otherwise all the ASM code is disabled by
# macros.
set_source_files_properties(${Sources} PROPERTIES LANGUAGE C)

add_library(zstd STATIC ${Sources} ${Headers})
add_library(zstd::static ALIAS zstd)

add_zstd_compilation_flags(zstd)

# Define library directory, where sources and header files are located
target_include_directories(zstd PUBLIC ${LIBRARY_DIR})
target_include_directories(zstd PRIVATE ${LIBRARY_DIR}/common)

if (ZSTD_LEGACY_SUPPORT)
	target_include_directories(zstd PRIVATE ${LIBRARY_DIR}/legacy)
	target_compile_definitions(zstd PRIVATE ZSTD_LEGACY_SUPPORT=${ZSTD_LEGACY_LEVEL})
else()
	target_compile_definitions(zstd PRIVATE ZSTD_LEGACY_SUPPORT=0)
endif()

if (ZSTD_MULTITHREAD_SUPPORT)
	target_compile_definitions(zstd PRIVATE ZSTD_MULTITHREAD)
	if (UNIX)
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
		target_link_libraries(zstd Threads::Threads)
		if (NOT CMAKE_USE_PTHREADS_INIT)
			message(SEND_ERROR "ZSTD currently does not support thread libraries other than pthreads")
		endif()
	endif()
endif()

# Add specific compile definitions for MSVC project
if (MSVC)
	target_compile_definitions(zstd PRIVATE ZSTD_HEAPMODE=0 ZSTD_DISABLE_ASM _CRT_SECURE_NO_WARNINGS)
endif()

# Define static library names
set_property(TARGET zstd PROPERTY POSITION_INDEPENDENT_CODE ON)
