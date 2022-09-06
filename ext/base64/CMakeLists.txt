add_library(base64
	base64.cpp base64.h)
add_library(External::base64 ALIAS base64)
target_include_directories(base64
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
