add_library(base64
	base64.cpp base64.h)
add_library(base64::base64 ALIAS base64)
set_target_properties(base64 PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_options(base64 PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
target_include_directories(base64
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
