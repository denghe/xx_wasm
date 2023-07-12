cmake_minimum_required(VERSION 3.20)

set(name hello_world)
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
