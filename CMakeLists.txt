cmake_minimum_required(VERSION 2.8.8)

#to set the version number directly with project
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

project(ZIMPL
    VERSION 3.3.3
    LANGUAGES C)

option(MT "use static runtime libraries for Visual Studio compiler" OFF)
option(GMP "use GMP" ON)
option(ZLIB "use ZLIB" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ZIMPL_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${ZIMPL_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ZIMPL_BINARY_DIR})

add_definitions(-DVERSION=\"${ZIMPL_VERSION_MAJOR}.${ZIMPL_VERSION_MINOR}.${ZIMPL_VERSION_PATCH}\")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# update the cmake submodule
if(EXISTS ${PROJECT_SOURCE_DIR}/.git AND NOT EXISTS ${PROJECT_SOURCE_DIR}/cmake/.git)
   find_program(GIT git)
   if(GIT)
      execute_process(
         COMMAND ${GIT} submodule update --init ${PROJECT_SOURCE_DIR}/cmake)
   endif()
endif()

# path to e.g. findGMP module
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

# make 'Release' the default build type
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# set MT or MTd flag when using the static VS runtime libraries
if(MSVC AND MT)
    set(CMAKE_C_FLAGS_RELEASE "/MT")
    set(CMAKE_C_FLAGS_DEBUG "/MTd")
endif()

if(GMP)
    find_package(GMP)
endif()
if(GMP_FOUND)
    include_directories(${GMP_INCLUDE_DIRS})
    set(libs ${libs} ${GMP_LIBRARIES})
endif()

if(ZLIB)
    find_package(ZLIB)
endif()
if(ZLIB_FOUND)
    set(libs ${libs} ${ZLIB_LIBRARIES})
    include_directories(${ZLIB_INCLUDE_DIRS})
else()
    add_definitions(-DWITHOUT_ZLIB)
endif()

include(CheckSymbolExists)

# check if we can use exp() with just including math.h
unset(HAVE_EXP CACHE)
check_symbol_exists(exp "math.h" HAVE_EXP)
if(NOT HAVE_EXP)
    # if not we link libm and check again
    unset(HAVE_EXP CACHE)
    list(APPEND CMAKE_REQUIRED_LIBRARIES m)
    check_symbol_exists(exp "math.h" HAVE_EXP)
    if(HAVE_EXP)
        set(libs ${libs} m)
    else()
        message(FATAL_ERROR "No exp() found")
    endif()
endif()

add_subdirectory(src)
