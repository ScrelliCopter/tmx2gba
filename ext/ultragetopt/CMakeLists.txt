include(CheckIncludeFile)
include(CheckFunctionExists)

check_include_file(strings.h HAVE_STRINGS_H)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(_stricmp HAVE__STRICMP)
check_function_exists(strncasecmp HAVE_STRNCASECMP)
check_function_exists(_strnicmp HAVE__STRNICMP)

add_library(ultragetopt
	ultragetopt.c ultragetopt.h)
add_library(External::ultragetopt ALIAS ultragetopt)

target_include_directories(ultragetopt
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(ultragetopt PRIVATE
	$<$<BOOL:${HAVE_STRINGS_H}>:HAVE_STRINGS_H=1>
	$<$<BOOL:${HAVE_STRCASECMP}>:HAVE_STRCASECMP=1>
	$<$<BOOL:${HAVE__STRICMP}>:HAVE__STRICMP=1>
	$<$<BOOL:${HAVE_STRNCASECMP}>:HAVE_STRNCASECMP=1>
	$<$<BOOL:${HAVE__STRNICMP}>:HAVE__STRNICMP=1>)
