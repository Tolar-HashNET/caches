cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(caches)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CACHE_INCLUDE_DIRS ${caches_SOURCE_DIR}/include CACHE INTERNAL "")

option(BUILD_TEST "Build tests for the project" ON)

if (${BUILD_TEST})
  enable_testing()
endif()

add_subdirectory(deps)
add_subdirectory(src)

if (${BUILD_TEST})
  add_subdirectory(test)
endif()
