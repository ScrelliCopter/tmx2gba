project(tmxlite VERSION 1.3.1)

# includes the list of source files in the src directory
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB PROJECT_SRC            ${PROJECT_DIR}/*.cpp)
file(GLOB PROJECT_SRC_DETAIL     ${PROJECT_DIR}/detail/*.cpp)
file(GLOB PROJECT_HEADERS        ${CMAKE_CURRENT_SOURCE_DIR}/include/tmxlite/*.hpp)
file(GLOB PROJECT_HEADERS_INL    ${CMAKE_CURRENT_SOURCE_DIR}/include/tmxlite/*.inl)
file(GLOB PROJECT_HEADERS_DETAIL ${CMAKE_CURRENT_SOURCE_DIR}/include/tmxlite/detail/*.hpp)
list(APPEND PROJECT_SRC ${PROJECT_SRC_DETAIL} ${PROJECT_HEADERS} ${PROJECT_HEADERS_INL} ${PROJECT_HEADERS_DETAIL})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON)

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	$<$<BOOL:${MSVC}>:_CRT_SECURE_NO_WARNINGS>  # disable msvc warning
	$<$<TARGET_EXISTS:ZLIB::ZLIB>:USE_ZLIB>)

target_link_libraries(${PROJECT_NAME} base64::base64 pugixml Zstd::Zstd
	$<$<TARGET_EXISTS:ZLIB::ZLIB>:ZLIB::ZLIB>
	$<$<TARGET_EXISTS:miniz::miniz>:miniz::miniz>)
