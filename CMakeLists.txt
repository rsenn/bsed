set(CMAKE_LEGACY_CYGWIN_WIN32 0) 
cmake_minimum_required(VERSION 2.8)

project(bsed)

include(CheckIncludeFile)

check_include_file(getopt.h HAVE_GETOPT_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(alloca.h HAVE_ALLOCA_H)
check_include_file(string.h HAVE_STRING_H)

if(HAVE_ALLOCA_H)
	add_definitions(-DHAVE_ALLOCA_H=${HAVE_ALLOCA_H})
endif()

if(HAVE_STRING_H)
	add_definitions(-DHAVE_STRING_H=${HAVE_STRING_H})
endif()

if(NOT HAVE_GETOPT_H)
  set(GETOPT_SOURCES getopt.c getopt.h)
endif()

include_directories("${CMAKE_SOURCE_DIR}")

#include(CheckTypeSize)
#
#check_type_size(loff_t LOFF_T)
#check_type_size(lloff_t LLOFF_T)
#check_type_size(off64_t OFF64_T)

include(CheckFunctionExists)

if(MINGW)
  set(HAVE_ALLOCA_FUNC TRUE)
else()
check_function_exists(alloca HAVE_ALLOCA_FUNC)
endif()

check_function_exists(geteuid HAVE_GETEUID_FUNC)
check_function_exists(chown HAVE_CHOWN_FUNC)

if(NOT HAVE_ALLOCA_FUNC)
  add_definitions(-DNO_ALLOCA_FUNC=1)
endif()

if(NOT HAVE_GETEUID_FUNC)
  add_definitions(-DNO_GETEUID_FUNC=1)
endif()

if(NOT HAVE_CHOWN_FUNC)
  add_definitions(-DNO_CHOWN_FUNC=1)
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -wd4996)
endif()

#configure_file("${CMAKE_SOURCE_DIR}/config.h.cmake"
#               "${CMAKE_BINARY_DIR}/config.h")
#

add_executable(bsed bsed.c ${GETOPT_SOURCES})
install(TARGETS bsed DESTINATION bin) 
