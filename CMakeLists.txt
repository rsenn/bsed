set(CMAKE_LEGACY_CYGWIN_WIN32 0) 
cmake_minimum_required(VERSION 2.8)

project(bsed)

include(CheckIncludeFile)

#check_include_file(inttypes.h HAVE_INTTYPES_H)
#check_include_file(stdint.h HAVE_STDINT_H)
#check_include_file(libgen.h HAVE_LIBGEN_H)
#check_include_file(alloca.h HAVE_ALLOCA_H)
#check_include_file(sys/stat.h HAVE_SYS_STAT_H)
#check_include_file(boost/foreach.hpp HAVE_BOOST_FOREACH_HPP)
#check_include_file(xtr1common HAVE_XTR1COMMON)
#check_include_file(initializer_list HAVE_INITIALIZER_LIST)
#
#include(CheckTypeSize)
#
#check_type_size(loff_t LOFF_T)
#check_type_size(lloff_t LLOFF_T)
#check_type_size(off64_t OFF64_T)
#
#include(CheckFunctionExists)
#
#check_function_exists(alloca HAVE_ALLOCA_FUNC)
#check_function_exists(stat HAVE_STAT_FUNC)
#check_function_exists(lstat HAVE_LSTAT_FUNC)
#check_function_exists(lseek HAVE_LSEEK_FUNC)
#check_function_exists(llseek HAVE_LLSEEK_FUNC)
#check_function_exists(lseek64 HAVE_LSEEK64_FUNC)
#
#configure_file("${CMAKE_SOURCE_DIR}/config.h.cmake"
#               "${CMAKE_BINARY_DIR}/config.h")
#

add_executable(bsed bsed.c)
install(TARGETS bsed DESTINATION bin) 
