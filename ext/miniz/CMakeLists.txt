add_library(miniz
	miniz.c miniz.h)
add_library(miniz::miniz ALIAS miniz)
target_include_directories(miniz
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
