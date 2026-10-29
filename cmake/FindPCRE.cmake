find_path(PCRE_INCLUDE_DIRS
    NAMES pcre.h pcre2.h pcre2posix.h
    HINTS ${PCRE_DIR}
    PATH_SUFFIXES include)

# on Windows, PCRE debug libs have an extra 'd' attached to the name
if(MSVC AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
   set(DEBUGd d)
endif()

find_library(PCRE_LIBRARY
   NAMES libpcre pcre2-posix${DEBUGd} pcre2-posix-static${DEBUGd}
   HINTS ${PCRE_DIR}
   PATH_SUFFIXES lib)

find_library(PCRE_LIBRARY_WIN
   NAMES pcre2-8${DEBUGd} pcre2-8-static${DEBUGd}
   HINTS ${PCRE_DIR}
   PATH_SUFFIXES lib)

SET(PCRE_LIBRARIES ${PCRE_LIBRARY} ${PCRE_LIBRARY_WIN})

if(${PCRE_LIBRARY} AND ${PCRE_INCLUDE_DIRS})
   set(PCRE_FOUND true)
else()
   set(PCRE_FOUND false)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE DEFAULT_MSG PCRE_INCLUDE_DIRS PCRE_LIBRARIES)
