project(tmxlite VERSION 1.3.1)

add_library(${PROJECT_NAME} STATIC
	include/tmxlite/Config.hpp
	include/tmxlite/FreeFuncs.hpp
	include/tmxlite/ImageLayer.hpp
	include/tmxlite/Layer.hpp
	include/tmxlite/LayerGroup.hpp
	include/tmxlite/Map.hpp
	include/tmxlite/Object.hpp
	include/tmxlite/ObjectGroup.hpp
	include/tmxlite/ObjectTypes.hpp
	include/tmxlite/Property.hpp
	include/tmxlite/TileLayer.hpp
	include/tmxlite/Tileset.hpp
	include/tmxlite/Types.hpp
	include/tmxlite/Types.inl
	include/tmxlite/detail/Log.hpp

	src/FreeFuncs.cpp
	src/ImageLayer.cpp
	src/Map.cpp
	src/Object.cpp
	src/ObjectGroup.cpp
	src/Property.cpp
	src/TileLayer.cpp
	src/LayerGroup.cpp
	src/Tileset.cpp
	src/ObjectTypes.cpp)

if (NOT USE_ZLIB)
	target_sources(${PROJECT_NAME} PRIVATE
		include/tmxlite/detail/gzip.hpp
		src/detail/gzip.cpp)
endif()

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
