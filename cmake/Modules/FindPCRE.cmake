# check whether we compile for x86 or x64
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(PCRE_ARCH_DIR "x64")
else()
  set(PCRE_ARCH_DIR "x86")
endif()

# check whether we compile for opt or debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(PCRE_BUILD_TYPE "debug")
else()
  set(PCRE_BUILD_TYPE "release")
endif()

find_path(PCRE_INCLUDE_DIRS
    NAMES pcre.h
    HINTS ${PCRE_DIR})

find_library(PCRE_LIBRARY
   NAMES libpcre
   HINTS ${PCRE_DIR}
   PATH_SUFFIXES lib/${PCRE_ARCH_DIR}/${PCRE_BUILD_TYPE})

SET(PCRE_LIBRARIES ${PCRE_LIBRARY})

if(${PCRE_LIBRARY} AND ${PCRE_INCLUDE_DIRS})
   set(PCRE_FOUND true)
else()
   set(PCRE_FOUND false)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE DEFAULT_MSG PCRE_INCLUDE_DIRS PCRE_LIBRARIES)
