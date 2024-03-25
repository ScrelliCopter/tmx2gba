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

add_library(pugixml-static STATIC
	${PROJECT_SOURCE_DIR}/src/pugixml.cpp)
add_library(pugixml::static ALIAS pugixml-static)

if (NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
	set_property(TARGET pugixml-static PROPERTY CXX_STANDARD_REQUIRED ON)
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD)
	set_property(TARGET pugixml-static PROPERTY CXX_STANDARD 11)
endif()

set_property(TARGET pugixml-static PROPERTY EXPORT_NAME static)
target_include_directories(pugixml-static PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
target_compile_definitions(pugixml-static PUBLIC
	${PUGIXML_BUILD_DEFINES}
	${PUGIXML_PUBLIC_DEFINITIONS})

add_library(pugixml INTERFACE)
target_link_libraries(pugixml INTERFACE pugixml-static)
add_library(pugixml::pugixml ALIAS pugixml)

set_target_properties(pugixml-static PROPERTIES
	EXCLUDE_FROM_ALL ON
	POSITION_INDEPENDENT_CODE ON
	SOVERSION ${PROJECT_VERSION_MAJOR}
	VERSION ${PROJECT_VERSION}
	OUTPUT_NAME pugixml)

set_target_properties(pugixml-static PROPERTIES
	EXCLUDE_FROM_ALL OFF)
