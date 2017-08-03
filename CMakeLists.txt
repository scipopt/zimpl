cmake_minimum_required(VERSION 3.0.0)

project(ZIMPL
    VERSION 3.3.4
    LANGUAGES C)

option(GMP "use GMP" ON)
option(ZLIB "use ZLIB" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ZIMPL_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${ZIMPL_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ZIMPL_BINARY_DIR}/lib)

add_definitions(-DVERSION=\"${ZIMPL_VERSION_MAJOR}.${ZIMPL_VERSION_MINOR}.${ZIMPL_VERSION_PATCH}\")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# path to e.g. findGMP module
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

# make 'Release' the default build type
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# filter /MD and /MDd from C and CXX flags
if(MSVC)
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )

    foreach(variable ${variables})
       string(REGEX REPLACE "/M[T,D][ d]" "" ${variable} "${${variable}}")
       message("${variable} = ${${variable}}")
    endforeach()
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

    # look for pcre if ZLIB could not be found
    find_package(PCRE)
    if(PCRE_FOUND)
        add_definitions(-DWITH_PCRE=1)
        set(libs ${libs} ${PCRE_LIBRARIES})
        include_directories(${PCRE_INCLUDE_DIRS})
    endif()
endif()

if(MSVC)
   add_definitions(-Dpopen=_popen)
   add_definitions(-Dpclose=_pclose)
   add_definitions(-Dinline=_inline)
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
