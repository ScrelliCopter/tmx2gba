# SPDX-License-Identifier: Zlib
# SPDX-FileCopyrightText: 2024 a dinosaur

#[=======================================================================[.rst:
FindZstd
--------

Find the Facebook Zstd library.

Imported Targets
^^^^^^^^^^^^^^^^

.. variable:: Zstd::Zstd

  :prop_tgt:`IMPORTED` target for using Zstd, if Zstd is found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:
.. variable:: Zstd_FOUND

  True if Zstd was found.

.. variable:: Zstd_INCLUDE_DIRS

  Path to the directory containing the Zstd headers (zstd.h, etc.)

.. variable:: Zstd_LIBRARIES

  Location of the Zstd library.

.. variable:: Zstd_VERSION

  The version of Zstd found.

Legacy Variables
^^^^^^^^^^^^^^^^

The following variables are defined by the official Zstd CMakeLists.txt:

.. variable:: zstd_VERSION_MAJOR

  The major version of Zstd.

.. variable:: zstd_VERSION_MINOR

  The minor version of Zstd.

.. variable:: zstd_VERSION_PATCH

  The patch/release version of Zstd.

The following variables are provided for compatibility with old find modules:

.. variable:: ZSTD_INCLUDE_DIR

  Directory containing the Zstd header. (use ``Zstd_INCLUDE_DIRS`` instead)

.. variable:: ZSTD_LIBRARY

  The Zstd library. (use ``Zstd_LIBRARIES`` instead)

Hints
^^^^^

.. variable:: Zstd_PREFER_STATIC_LIBS

  Set to ``ON`` to prefer static libraries. Defaults to ``OFF``

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set,
these are transitory and should not be relied upon:

.. variable:: Zstd_INCLUDE_DIR

  Directory containing the Zstd header.

.. variable:: Zstd_LIBRARY_DEBUG

  The Zstd debug library if found.

.. variable:: Zstd_LIBRARY_RELEASE

  The Zstd release library if found.

.. variable:: Zstd_LIBRARY

  The Zstd library.

#]=======================================================================]

#TODO: define Zstd::static & Zstd::shared and alias Zstd::Zstd based on preference

find_path(Zstd_INCLUDE_DIR NAMES zstd.h)

mark_as_advanced(Zstd_INCLUDE_DIR)

if (Zstd_PREFER_STATIC_LIBS)
	find_library(Zstd_LIBRARY_DEBUG NAMES zstd_staticd zstdd)
	find_library(Zstd_LIBRARY_RELEASE NAMES zstd_static zstd)
else()
	find_library(Zstd_LIBRARY_DEBUG NAMES zstdd zstd_staticd)
	find_library(Zstd_LIBRARY_RELEASE NAMES zstd zstd_static)
endif()

include(SelectLibraryConfigurations)
select_library_configurations(Zstd)

mark_as_advanced(Zstd_LIBRARY Zstd_LIBRARY_DEBUG Zstd_LIBRARY_RELEASE)

if (Zstd_INCLUDE_DIR AND EXISTS "${Zstd_INCLUDE_DIR}/zstd.h")
	function (_zstd_read_define _variable _define)
		set(_file "${Zstd_INCLUDE_DIR}/zstd.h")
		set(_regex "#define[ \t]+${_define}[ \t]+([0-9]+)")
		file(STRINGS "${_file}" _line LIMIT_COUNT 1 REGEX "${_regex}")
		if (CMAKE_VERSION VERSION_LESS "3.29")
			string(REGEX MATCH "${_regex}" _line "${_line}")
		endif()
		set(${_variable} ${CMAKE_MATCH_1} PARENT_SCOPE)
	endfunction()

	_zstd_read_define(zstd_VERSION_MAJOR "ZSTD_VERSION_MAJOR")
	_zstd_read_define(zstd_VERSION_MINOR "ZSTD_VERSION_MINOR")
	_zstd_read_define(zstd_VERSION_PATCH "ZSTD_VERSION_RELEASE")
	set(Zstd_VERSION "${zstd_VERSION_MAJOR}.${zstd_VERSION_MINOR}.${zstd_VERSION_PATCH}")

	mark_as_advanced(zstd_VERSION_MAJOR zstd_VERSION_MINOR zstd_VERSION_PATCH Zstd_VERSION)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Zstd
	REQUIRED_VARS Zstd_LIBRARY Zstd_INCLUDE_DIR
	VERSION_VAR Zstd_VERSION)

mark_as_advanced(Zstd_FOUND)

if (Zstd_FOUND)
	set(Zstd_INCLUDE_DIRS ${Zstd_INCLUDE_DIR})
	set(Zstd_LIBRARIES ${Zstd_LIBRARY})

	# Legacy variables
	set(ZSTD_INCLUDE_DIR ${Zstd_INCLUDE_DIR})
	set(ZSTD_LIBRARY ${Zstd_LIBRARY})
	mark_as_advanced(ZSTD_INCLUDE_DIR ZSTD_LIBRARY)

	if (NOT TARGET Zstd::Zstd)
		add_library(Zstd::Zstd UNKNOWN IMPORTED)
		set_property(TARGET Zstd::Zstd PROPERTY
			INTERFACE_INCLUDE_DIRECTORIES "${Zstd_INCLUDE_DIR}")
	endif()

	if (NOT Zstd_LIBRARY_DEBUG AND NOT Zstd_LIBRARY_RELEASE)
		set_property(TARGET Zstd::Zstd PROPERTY
			IMPORTED_LOCATION "${Zstd_LIBRARY}")
	endif()
	if (Zstd_LIBRARY_DEBUG)
		set_property(TARGET Zstd::Zstd APPEND PROPERTY
			IMPORTED_CONFIGURATIONS DEBUG)
		set_property(TARGET Zstd::Zstd PROPERTY
			IMPORTED_LOCATION_DEBUG "${Zstd_LIBRARY_DEBUG}")
	endif()
	if (Zstd_LIBRARY_RELEASE)
		set_property(TARGET Zstd::Zstd APPEND PROPERTY
			IMPORTED_CONFIGURATIONS RELEASE)
		set_property(TARGET Zstd::Zstd PROPERTY
			IMPORTED_LOCATION_RELEASE "${Zstd_LIBRARY_RELEASE}")
	endif()
endif()
