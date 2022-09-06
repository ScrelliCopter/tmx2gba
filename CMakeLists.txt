cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(tmx2gba)

# C++11 & C99
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

# Enable strong warnings
if (MSVC)
	string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REPLACE "/W3" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
else()
	add_compile_options(-Wall)
endif()

# Libraries
add_subdirectory(ext/base64)
add_subdirectory(ext/miniz)
add_subdirectory(ext/rapidxml)
add_subdirectory(ext/ultragetopt)

# Main tmx2gba sources
add_subdirectory(src)

if (MSVC)
	# Default to tmx2gba as startup project when generating Solutions
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT tmx2gba)
endif()
