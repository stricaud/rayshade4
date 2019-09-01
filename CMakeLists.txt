cmake_minimum_required(VERSION 2.8)

project(rayshade)
message("CMake system name: ${CMAKE_SYSTEM_NAME}")

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
message(FATAL_ERROR "CMake generation is not allowed within the source directory!
Remove the CMakeCache.txt file and try again from another folder, e.g.:

   rm CMakeCache.txt
   mkdir build
   cd build
   cmake ..
")
endif()


set(FAUP_INCLUDE_DIRS "${rayshade_SOURCE_DIR}/src/lib/include/" "${rayshade_BINARY_DIR}/src/lib/include")
set(FAUP_LIBRARY faupl)

add_subdirectory(src)
