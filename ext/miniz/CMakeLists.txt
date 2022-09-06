add_library(miniz INTERFACE)
add_library(External::miniz ALIAS miniz)
target_include_directories(miniz
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
