project(tmxlite VERSION 1.3.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(USE_RTTI TRUE CACHE BOOL "Use run time type information?")

set(USE_EXTLIBS FALSE CACHE BOOL "Use external zlib, zstd and pugixml libraries instead of the included source?")
set(USE_ZSTD FALSE CACHE BOOL "Enable zstd compression? (Already set to true if USE_EXTLIBS is true)")

# includes the list of source files in the src directory
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB PROJECT_SRC ${PROJECT_DIR}/*.cpp)

add_library(tmxlite STATIC ${PROJECT_SRC})

set_target_properties(tmxlite PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

target_compile_definitions(tmxlite PRIVATE $<$<CONFIG:Debug>:_DEBUG_> TMXLITE_STATIC)
target_compile_options(tmxlite PRIVATE -Wall)
if (NOT USE_RTTI AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
	target_compile_options(tmxlite PRIVATE -fno-rtti)
endif()

# disable msvc warning
if (MSVC)
	target_compile_definitions(tmxlite PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# if we want external zip and xml libs find them and tell the compiler
if (USE_EXTLIBS)
	find_package(ZLIB REQUIRED)
	find_package(PUGIXML REQUIRED)
	find_package(Zstd REQUIRED)

	target_compile_definitions(tmxlite PRIVATE USE_EXTLIBS USE_ZSTD)
	target_include_directories(tmxlite PRIVATE ${ZLIB_INCLUDE_DIRS} ${PUGIXML_INCLUDE_DIR} ${ZSTD_INCLUDE_DIR})
	target_link_libraries(tmxlite ${ZLIB_LIBRARIES} ${PUGIXML_LIBRARY} ${ZSTD_LIBRARY})
else()
	# add miniz and pugixml from source
	target_link_libraries(tmxlite pugixml::static miniz::miniz)

	if (USE_ZSTD)
		target_compile_definitions(tmxlite PRIVATE USE_ZSTD)
		target_link_libraries(tmxlite zstd::static)
	endif()
endif()

target_include_directories(tmxlite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
