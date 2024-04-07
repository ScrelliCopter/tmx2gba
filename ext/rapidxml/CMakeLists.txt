add_library(rapidxml INTERFACE)
add_library(External::rapidxml ALIAS rapidxml)
target_include_directories(rapidxml
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
