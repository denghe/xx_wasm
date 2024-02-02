cmake_minimum_required(VERSION 3.20)

set(name openal)
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

macro(append_linker_flags FLAGS)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()

append_linker_flags("-lopenal")

if(MSVC)
	include_directories(
		$ENV{EMSDK}/upstream/emscripten/system/lib/libcxx/include
		$ENV{EMSDK}/upstream/emscripten/system/include
		$ENV{EMSDK}/upstream/emscripten/system/lib/libc/musl/include
		$ENV{EMSDK}/upstream/emscripten/system/lib/libc/musl/arch/emscripten
	)
endif()
