cmake_minimum_required(VERSION 3.20)

set(name hello_world_sdl)
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

source_group("" FILES ${SRCS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})

macro(append_linker_flags FLAGS)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")

	# Link in the JS library file for support code
	#append_linker_flags("--js-library ${CMAKE_CURRENT_LIST_DIR}/library_js.js")

	# Optimization flag to optimize aggressively for size. (other options -Os, -O3, -O2, -O1, -O0)
	#append_linker_flags("-O0")
	#append_linker_flags("-g")
	append_linker_flags("-Os")
	
	append_linker_flags("-pthread")
	#append_linker_flags("-sALLOW_MEMORY_GROWTH=1")
	append_linker_flags("-sTOTAL_MEMORY=134217728")
	append_linker_flags("-sASSERTIONS=1")
	append_linker_flags("-sNO_DISABLE_EXCEPTION_CATCHING=1")
	
	append_linker_flags("-sTEXTDECODER=2")
	
	# The generated build output is only to be expected to be run in a web browser, never in a native CLI shell, or in a web worker.
	append_linker_flags("-sENVIRONMENT=web,worker")
	
	append_linker_flags("-sERROR_ON_UNDEFINED_SYMBOLS=0")
	
	# Enable aggressive MINIMAL_RUNTIME mode.
	append_linker_flags("-sMINIMAL_RUNTIME=2")

	# Enable Closure compiler for aggressive JS size minification
	append_linker_flags("--closure 1")

	# Reduce code size: We do not need libc errno field support in our build output.
	append_linker_flags("-sSUPPORT_ERRNO=0")

	# Reduce code size: We do not need native POSIX filesystem emulation support (Emscripten FS/MEMFS)
	append_linker_flags("-sFILESYSTEM=0")
	
	# Choose the oldest browser versions that should be supported. The higher minimum bar you choose, the less
	# emulation code may be present for old browser quirks.
	append_linker_flags("-sMIN_FIREFOX_VERSION=79")
	append_linker_flags("-sMIN_SAFARI_VERSION=150000")
	append_linker_flags("-sMIN_IE_VERSION=0x7FFFFFFF") # Do not support Internet Explorer at all (this is the Emscripten default, shown here for posterity)
	append_linker_flags("-sMIN_EDGE_VERSION=79") # Require Chromium-based Edge browser
	append_linker_flags("-sMIN_CHROME_VERSION=80")

	# Link in to WebGL/GLES system library
	append_linker_flags("-lGL")
	append_linker_flags("-lglfw")
	append_linker_flags("-sUSE_GLFW=3")
	append_linker_flags("-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3")
	add_definitions(-DMAX_WEBGL_VERSION=2)
	
	# Fine tuning for code size: do not generate code to abort program execution on malloc() failures, that will
	# not be interesting here.
	append_linker_flags("-sABORTING_MALLOC=0")

	# Reduce WebGL code size at the expense of performance (this only has an effect in WebGL 1, practically a no-op here)
	append_linker_flags("-sGL_POOL_TEMP_BUFFERS=0")

	# Reduce WebGL code size: WebGL bindings layer should not keep track of certain WebGL
	# errors that are only meaningful for C/C++ applications. (good to enable for release when glGetError() is not used, but disable in debug)
	append_linker_flags("-sGL_TRACK_ERRORS=0")

endif()

#if (EMSCRIPTEN)
#	set_target_properties(${name} PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_LIST_DIR}/res@res")
#endif()
