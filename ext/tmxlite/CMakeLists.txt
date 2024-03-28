project(tmxlite VERSION 1.3.1)

set(USE_RTTI TRUE CACHE BOOL "Use run time type information?")

# includes the list of source files in the src directory
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB PROJECT_SRC            ${PROJECT_DIR}/*.cpp)
file(GLOB PROJECT_HEADERS        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB PROJECT_HEADERS_INL    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl)
file(GLOB PROJECT_HEADERS_DETAIL ${CMAKE_CURRENT_SOURCE_DIR}/include/detail/*.hpp)
set(PROJECT_SRC ${PROJECT_SRC} ${PROJECT_HEADERS} ${PROJECT_HEADERS_INL} ${PROJECT_HEADERS_DETAIL})

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

target_link_libraries(tmxlite base64::base64)

if (USE_BUNDLED_PUGIXML)
	target_link_libraries(tmxlite pugixml::static)
else()
	target_include_directories(tmxlite PRIVATE ${PUGIXML_INCLUDE_DIR})
	target_link_libraries(tmxlite ZLIB::ZLIB ${PUGIXML_LIBRARY})
endif()

if (USE_ZLIB)
	target_compile_definitions(tmxlite PRIVATE USE_ZLIB)
	target_link_libraries(tmxlite ZLIB::ZLIB)
else()
	target_link_libraries(tmxlite miniz::miniz)
endif()

target_compile_definitions(tmxlite PRIVATE USE_ZSTD)
target_link_libraries(tmxlite Zstd::Zstd)

target_include_directories(tmxlite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
