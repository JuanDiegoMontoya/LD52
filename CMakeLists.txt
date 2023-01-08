cmake_minimum_required(VERSION 3.15)

project(LD52)

set(CMAKE_CXX_STANDARD 20)

set(LD52_source_files
	"src/main.cpp"
 "src/Application.cpp" )

set(LD52_header_files
	"src/main.h"
 "src/Application.h" )

add_executable(LD52_game
	${LD52_source_files}
	${LD52_header_files}
)

# Determine whether we're compiling with clang++
string(FIND "${CMAKE_CXX_COMPILER}" "clang++" COMPILER_CLANGPP)
if(COMPILER_CLANGPP GREATER -1)
    set(COMPILER_CLANGPP 1)
else()
    set(COMPILER_CLANGPP 0)
endif()

# enable roughly equivalent (and strict!) compiler warnings depending on the compiler
target_compile_options(LD52_game
	INTERFACE
	$<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>,${COMPILER_CLANGPP}>:
	-Wall
	-Wextra
	-pedantic-errors
	-Wno-missing-field-initializers
	-Wno-unused-result
	#-Werror
	#-Wconversion
	#-Wsign-conversion
	>
	$<$<CXX_COMPILER_ID:MSVC>:
	/W4
	/WX
	/permissive-
	/wd4324 # structure was padded
	>
)

find_package(OpenGL REQUIRED)

# enable asan for debug builds
if (DEBUG)
    target_compile_options(LD52_game PUBLIC -fsanitize=address)
endif()

add_subdirectory(external)

target_include_directories(LD52_game PUBLIC	src)

target_link_libraries(LD52_game glm EnTT::EnTT fwog glfw lib_glad imgui stb)

add_custom_target(copy_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(LD52_game copy_assets)