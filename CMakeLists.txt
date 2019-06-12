cmake_minimum_required(VERSION 3.13.0)
project(otypes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)

include_directories(include)

file(GLOB_RECURSE otypes_SRC
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	otypes STATIC
	${otypes_SRC}
)

if(MSVC)
  target_compile_options(otypes PRIVATE /W4 /WX /MD /MP)
else()
  target_compile_options(otypes PRIVATE -Wall -Wextra -pedantic -Werror)
endif()