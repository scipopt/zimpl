cmake_minimum_required(VERSION 3.3)

project(ZIMPL
    VERSION 3.5.0
    LANGUAGES C)

# if changing these flags, also update GCCWARN/GXXWARN in make/make.project
set(ADD_C_FLAGS "-Wall -Wextra -Wno-unknown-pragmas -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wmissing-noreturn -Wmissing-declarations -fno-omit-frame-pointer ")
set(ADD_SANITIZE_FLAGS "-fsanitize=address -fsanitize=undefined -fsanitize=shift -fsanitize=shift-exponent -fsanitize=shift-base -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=null -fsanitize=bounds -fsanitize=alignment -fsanitize=object-size -fsanitize=float-cast-overflow -fsanitize=nonnull-attribute -fsanitize=returns-nonnull-attribute -fsanitize=bool -fsanitize=enum -fsanitize=signed-integer-overflow")

if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(ADD_GNU_FLAGS "-Wsuggest-attribute=format")
    if (CMAKE_C_COMPILER_VERSION VERSION_LESS 5)
        message(WARNING "GCC version smaller than 5, it is recommended to use version 10 or higher.")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_C_FLAGS} ${ADD_GNU_FLAGS} -fsanitize=address -fsanitize=thread -Wno-nonnull-compare -Wno-unused-parameter -Wno-unused-function -Wno-pragmas -Wno-clobbered -Wno-attributes")
    elseif (CMAKE_C_COMPILER_VERSION VERSION_LESS 8)
        message(WARNING "GCC version smaller than 8, it is recommended to use version 10 or higher.")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_C_FLAGS} ${ADD_GNU_FLAGS} ${ADD_SANITIZE_FLAGS} -Wno-suggest-attribute=noreturn -Wno-nonnull-compare -Wno-unused-parameter -Wno-unused-function -fstack-protector-strong -Wduplicated-branches ")
    elseif (CMAKE_C_COMPILER_VERSION VERSION_LESS 9)
        message(WARNING "GCC version smaller than 9, it is recommended to use version 10 or higher.")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_C_FLAGS} ${ADD_GNU_FLAGS} ${ADD_SANITIZE_FLAGS} -Wno-suggest-attribute=noreturn -Wno-nonnull-compare -Wno-unused-parameter -Wno-unused-function -fstack-protector-strong -Wduplicated-branches ")
    else() # gcc 10 or later
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_C_FLAGS} ${ADD_GNU_FLAGS} ${ADD_SANITIZE_FLAGS} -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=pure -Wsuggest-attribute=malloc -Wsuggest-attribute=cold -Wstrict-overflow=4 -fstack-protector-strong -Wduplicated-branches -fsanitize=pointer-overflow -fsanitize=builtin -Wno-nonnull-compare -Wno-error=unknown-warning ") # additional parameters don't exist in gcc 7
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(ADD_CLANG_FLAGS "-Wno-unused-parameter -Wno-unused-function -Wno-cast-align -Wno-strict-prototypes -Wno-pragmas -Wno-attributes -Wstrict-overflow=4 -fstack-protector-strong -Wno-invalid-command-line-argument")
    set(EXTRA_CLANG_FLAGS "-Weverything -Wno-reserved-id-macro -Wno-cast-qual -Wno-bad-function-cast -Wno-float-equal -Wno-unused-macros -Wno-implicit-fallthrough -Wno-switch-enum -Wno-padded -Wno-covered-switch-default -Wno-shorten-64-to-32 -Wno-pedantic -Wno-format-nonliteral -Wno-disabled-macro-expansion -Wno-static-in-inline")
    if (CMAKE_C_COMPILER_VERSION VERSION_LESS 7)
        message(WARNING "Clang version smaller than 7, it is recommended to use version 10 or higher.")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_C_FLAGS} ${ADD_CLANG_FLAGS} ${ADD_SANITIZE_FLAGS} ${EXTRA_CLANG_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_C_FLAGS} ${ADD_CLANG_FLAGS} ${ADD_SANITIZE_FLAGS} ${EXTRA_CLANG_FLAGS} -Wno-extra-semi-stmt")
    endif()
endif()

# use C99 standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

option(ZLIB "use ZLIB" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_definitions(-DVERSION=\"${ZIMPL_VERSION_MAJOR}.${ZIMPL_VERSION_MINOR}.${ZIMPL_VERSION_PATCH}\")

# path to e.g. findGMP module
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(GMP REQUIRED)

# make 'Release' the default build type
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_C_COMPILER MATCHES "intel")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -wd111,151,171,279,981,1173,1419,1684,2259")
endif()

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
       # message("${variable} = ${${variable}}")
    endforeach()
endif()

include_directories(${GMP_INCLUDE_DIRS})
set(libs ${libs} ${GMP_LIBRARIES})

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
        add_definitions(-DWITH_PCRE)
        add_definitions(-DPCRE2_STATIC)
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

# link to math library if it is available
find_library(libm m)
if(NOT libm)
  set(libm "")
endif()

set(libs ${libs} ${libm})

add_subdirectory(src)
