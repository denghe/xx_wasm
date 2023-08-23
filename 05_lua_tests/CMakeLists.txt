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

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	target_precompile_headers(${name}
		PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>
	)

	if (${CMAKE_BUILD_TYPE} MATCHES "Rel*")
		target_link_options(${name} PUBLIC
			-lembind
			-sALLOW_MEMORY_GROWTH=1
			--preload-file "${CMAKE_CURRENT_LIST_DIR}/res@res"
		)
	else()
		target_link_options(${name} PUBLIC
			-lembind
			-sALLOW_MEMORY_GROWTH=1
			-sNO_DISABLE_EXCEPTION_CATCHING=1
			--preload-file "${CMAKE_CURRENT_LIST_DIR}/res@res"
		)
	endif()
endif()

if(MSVC)
	include_directories(
		$ENV{EMSDK}/upstream/emscripten/system/lib/libcxx/include
		$ENV{EMSDK}/upstream/emscripten/system/include
		$ENV{EMSDK}/upstream/emscripten/system/lib/libc/musl/include
		$ENV{EMSDK}/upstream/emscripten/system/lib/libc/musl/arch/emscripten
	)
endif()
