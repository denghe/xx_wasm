cmake_minimum_required(VERSION 3.20)

set(name lua_tests)
project(${name})

set(CMAKE_CXX_STANDARD 20)

include_directories(
	../xxlib
)

set(SRCS)
file(GLOB_RECURSE SRCS
	*.h
	*.hpp
	*.inc
	*.c
	*.cpp
)
add_executable(${name} ${SRCS})

set(CMAKE_EXECUTABLE_SUFFIX ".html")





add_definitions(-DMAKE_LIB)	# for lua
include_directories(
	../lua
)
set(SRCS)
file(GLOB SRCS	#GLOB_RECURSE
	../lua/onelua.c
)
add_library(lua ${SRCS})
target_link_libraries(${name} lua)
