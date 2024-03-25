project(tmxlite VERSION 1.3.1)
set(PROJECT_NAME tmxlite)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

if(NOT TMXLITE_STATIC_LIB)
  SET(TMXLITE_STATIC_LIB FALSE CACHE BOOL "Should tmxlite be built as a static or shared lib?")
endif()

SET(PROJECT_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries?")

SET(USE_RTTI TRUE CACHE BOOL "Use run time type information?")

SET(USE_EXTLIBS FALSE CACHE BOOL "Use external zlib, zstd and pugixml libraries instead of the included source?")
SET(USE_ZSTD FALSE CACHE BOOL "Enable zstd compression? (Already set to true if USE_EXTLIBS is true)")

if(USE_RTTI)
  if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
    if(PROJECT_STATIC_RUNTIME)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -static")
    else()
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    endif()
  endif()
else()
  if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
    if(PROJECT_STATIC_RUNTIME)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti -static")
    else()
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti")
    endif()
  endif()
endif()

if(TMXLITE_STATIC_LIB)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG_ -DTMXLITE_STATIC")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -DTMXLITE_STATIC")
  SET(CMAKE_DEBUG_POSTFIX -s-d)
  SET(CMAKE_RELEASE_POSTFIX -s)
else()
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG_")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
  SET(CMAKE_DEBUG_POSTFIX -d)
endif()

# includes the list of source files in the src directory
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include(${PROJECT_DIR}/CMakeLists.txt)

if(TMXLITE_STATIC_LIB)
	add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
else()
	add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

# disable msvc warning
if(MSVC)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# if we want external zip and xml libs find them and tell the compiler
if(USE_EXTLIBS)
	find_package(ZLIB REQUIRED)
	find_package(PUGIXML REQUIRED)
	find_package(Zstd REQUIRED)

	target_compile_definitions(${PROJECT_NAME} PRIVATE USE_EXTLIBS USE_ZSTD)
	target_include_directories(${PROJECT_NAME} PRIVATE ${ZLIB_INCLUDE_DIRS} ${PUGIXML_INCLUDE_DIR} ${ZSTD_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES} ${PUGIXML_LIBRARY} ${ZSTD_LIBRARY})
else()
	# add miniz and pugixml from source
	target_link_libraries(${PROJECT_NAME} pugixml::static External::miniz)

	if(USE_ZSTD)
		target_compile_definitions(${PROJECT_NAME} PRIVATE USE_ZSTD)
		target_link_libraries(${PROJECT_NAME} zstd::static)
	endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
