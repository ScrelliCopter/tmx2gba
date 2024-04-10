project(pugixml VERSION 1.14)

# Expose options from the pugiconfig.hpp
option(PUGIXML_WCHAR_MODE "Enable wchar_t mode" OFF)
option(PUGIXML_COMPACT "Enable compact mode" OFF)

# Advanced options from pugiconfig.hpp
option(PUGIXML_NO_XPATH "Disable XPath" OFF)
option(PUGIXML_NO_STL "Disable STL" OFF)
option(PUGIXML_NO_EXCEPTIONS "Disable Exceptions" OFF)
mark_as_advanced(PUGIXML_NO_XPATH PUGIXML_NO_STL PUGIXML_NO_EXCEPTIONS)

set(PUGIXML_PUBLIC_DEFINITIONS
	$<$<BOOL:${PUGIXML_WCHAR_MODE}>:PUGIXML_WCHAR_MODE>
	$<$<BOOL:${PUGIXML_COMPACT}>:PUGIXML_COMPACT>
	$<$<BOOL:${PUGIXML_NO_XPATH}>:PUGIXML_NO_XPATH>
	$<$<BOOL:${PUGIXML_NO_STL}>:PUGIXML_NO_STL>
	$<$<BOOL:${PUGIXML_NO_EXCEPTIONS}>:PUGIXML_NO_EXCEPTIONS>)

add_library(pugixml STATIC
	src/pugiconfig.hpp
	src/pugixml.hpp
	src/pugixml.cpp)
add_library(pugixml::static ALIAS pugixml)
add_library(pugixml::pugixml ALIAS pugixml)

set_target_properties(pugixml PROPERTIES
	CXX_STANDARD_REQUIRED ON
	CXX_STANDARD 11)

set_property(TARGET pugixml PROPERTY EXPORT_NAME static)
target_include_directories(pugixml PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
target_compile_definitions(pugixml PUBLIC
	${PUGIXML_BUILD_DEFINES}
	${PUGIXML_PUBLIC_DEFINITIONS})

add_library(pugixml::pugixml ALIAS pugixml)

set_target_properties(pugixml PROPERTIES
	EXCLUDE_FROM_ALL ON
	POSITION_INDEPENDENT_CODE ON
	SOVERSION ${PROJECT_VERSION_MAJOR}
	VERSION ${PROJECT_VERSION}
	OUTPUT_NAME pugixml)

set_target_properties(pugixml PROPERTIES
	EXCLUDE_FROM_ALL OFF)
