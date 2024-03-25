# ################################################################
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# ################################################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

set(LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Parse version
include(GetZstdLibraryVersion)
GetZstdLibraryVersion(${LIBRARY_DIR}/zstd.h zstd_VERSION_MAJOR zstd_VERSION_MINOR zstd_VERSION_PATCH)

enable_language(ASM)

#-----------------------------------------------------------------------------
# Add extra compilation flags
#-----------------------------------------------------------------------------
include(AddZstdCompilationFlags)
ADD_ZSTD_COMPILATION_FLAGS()

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
	set(LIBRARY_LEGACY_DIR ${LIBRARY_DIR}/legacy)

	set(Sources ${Sources}
		${LIBRARY_LEGACY_DIR}/zstd_v01.c
		${LIBRARY_LEGACY_DIR}/zstd_v02.c
		${LIBRARY_LEGACY_DIR}/zstd_v03.c
		${LIBRARY_LEGACY_DIR}/zstd_v04.c
		${LIBRARY_LEGACY_DIR}/zstd_v05.c
		${LIBRARY_LEGACY_DIR}/zstd_v06.c
		${LIBRARY_LEGACY_DIR}/zstd_v07.c)

	set(Headers ${Headers}
		${LIBRARY_LEGACY_DIR}/zstd_legacy.h
		${LIBRARY_LEGACY_DIR}/zstd_v01.h
		${LIBRARY_LEGACY_DIR}/zstd_v02.h
		${LIBRARY_LEGACY_DIR}/zstd_v03.h
		${LIBRARY_LEGACY_DIR}/zstd_v04.h
		${LIBRARY_LEGACY_DIR}/zstd_v05.h
		${LIBRARY_LEGACY_DIR}/zstd_v06.h
		${LIBRARY_LEGACY_DIR}/zstd_v07.h)
endif()

# Explicitly set the language to C for all files, including ASM files.
# Our assembly expects to be compiled by a C compiler, and is only enabled for
# __GNUC__ compatible compilers. Otherwise all the ASM code is disabled by
# macros.
set_source_files_properties(${Sources} PROPERTIES LANGUAGE C)

# Split project to static and shared libraries build
set(library_targets)
add_library(libzstd_shared SHARED ${Sources} ${Headers} ${PlatformDependResources})
add_library(zstd::shared ALIAS libzstd_shared)
list(APPEND library_targets libzstd_shared)

add_library(libzstd_static STATIC ${Sources} ${Headers})
add_library(zstd::static ALIAS libzstd_static)
list(APPEND library_targets libzstd_static)

foreach (LIB ${library_targets})
	# Define library directory, where sources and header files are located
	target_include_directories(${LIB} PUBLIC ${LIBRARY_DIR})
	target_include_directories(${LIB} PRIVATE ${LIBRARY_DIR}/common)

	if (ZSTD_LEGACY_SUPPORT)
		include_directories(${LIBRARY_LEGACY_DIR})
		target_compile_definitions(${LIB} PRIVATE ZSTD_LEGACY_SUPPORT=${ZSTD_LEGACY_LEVEL})
	else()
		target_compile_definitions(${LIB} PRIVATE ZSTD_LEGACY_SUPPORT=0)
	endif()
endforeach()

if (ZSTD_MULTITHREAD_SUPPORT)
	target_compile_definitions(libzstd_shared PRIVATE ZSTD_MULTITHREAD)
	target_compile_definitions(libzstd_static PRIVATE ZSTD_MULTITHREAD)
	if (UNIX)
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
		target_link_libraries(libzstd_shared Threads::Threads)
		target_link_libraries(libzstd_static Threads::Threads)
		if (NOT CMAKE_USE_PTHREADS_INIT)
			message(SEND_ERROR "ZSTD currently does not support thread libraries other than pthreads")
		endif()
	endif()
endif()

# Add specific compile definitions for MSVC project
if (MSVC)
	set(COMMON_DEFINITIONS ZSTD_DISABLE_ASM _CRT_SECURE_NO_WARNINGS)
	target_compile_definitions(libzstd_shared PRIVATE ZSTD_DLL_EXPORT=1 ZSTD_HEAPMODE=0 _CONSOLE ${COMMON_DEFINITIONS})
	target_compile_definitions(libzstd_static PRIVATE ZSTD_HEAPMODE=0 ${COMMON_DEFINITIONS})
endif()

# Define static and shared library names
set_target_properties(libzstd_shared PROPERTIES
	OUTPUT_NAME zstd
	VERSION ${zstd_VERSION_MAJOR}.${zstd_VERSION_MINOR}.${zstd_VERSION_PATCH}
	SOVERSION ${zstd_VERSION_MAJOR})

set_property(TARGET libzstd_static PROPERTY POSITION_INDEPENDENT_CODE ON)

# With MSVC static library needs to be renamed to avoid conflict with import library
if (MSVC OR (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT MINGW))
	set_property(TARGET libzstd_static PROPERTY OUTPUT_NAME zstd_static)
else()
	set_property(TARGET libzstd_static PROPERTY OUTPUT_NAME zstd)
endif()
