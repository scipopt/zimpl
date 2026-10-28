# Overall structure
#
# POLICIES
# PROJECT DEFINITIONS
#    Version
#    Project declaration
#    Minimum dependency versions
#    Directories and search paths
#    Packaging
# OPTIONS
#    Declarations with preprocessing/printing
# BASIC SETTINGS
#    Shared/static libraries
#    Build type
#    LTO
#    Githash
# DEPENDENCIES
#    Search and process
#    Check for position-independent code
# TOOLS
#    Code coverage.
#    Global compilation settings
# SUBDIRECTORIES
# INSTALLATION
#    Licenses
#    Config(-version) files
#    Build-tree targets
# LOGGING
#    Non-default parameters

cmake_minimum_required(VERSION 3.11)

project(ZIMPL
   VERSION 3.9.0
   LANGUAGES C)

# Sets installation directories according to GNU coding standards.
include(GNUInstallDirs)

# Path to CMake modules such as FindGMP.cmake.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

##### OPTIONS #####

# Whether to build a shared library.
option(BUILD_SHARED_LIBS "Whether to build a pic-enabled static library" ON)
# For compatibility with the Makefile build system, the SHARED option exists.
set(SHARED AUTO CACHE STRING "Whether to build a pic-enabled static library (AUTO: use BUILD_SHARED_LIBS)")
set_property(CACHE SHARED PROPERTY STRINGS AUTO ON OFF)
string(TOUPPER "${SHARED}" SHARED)

set(GMP AUTO CACHE STRING "should ZIMPL link to GMP")
set_property(CACHE GMP PROPERTY STRINGS AUTO ON OFF)
string(TOUPPER "${GMP}" GMP)

option(SANITIZE "should sanitizers be enabled if available" OFF)

set(ZLIB AUTO CACHE STRING "should ZIMPL link to ZLIB")
set_property(CACHE ZLIB PROPERTY STRINGS AUTO ON OFF)
string(TOUPPER "${ZLIB}" ZLIB)

##### BASIC SETTINGS ####

# Build type can be set using SHARED or, if SHARED=auto, then via BUILD_SHARED_LIBS, the common CMake way.
if(NOT SHARED)
   set(BUILD_SHARED_LIBS 0)
elseif(NOT (SHARED STREQUAL "AUTO"))
   set(BUILD_SHARED_LIBS 1)
endif()

if(BUILD_SHARED_LIBS)
  # We don't build a shared library because the xlpglue works using unresolved symbols.
   message(STATUS "Building static library with position-independent code.")
   set(ZIMPL_HAS_PIC 1)
   set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
   message(STATUS "Building static library.")
   set(ZIMPL_HAS_PIC 0)
endif()

# The default build type is 'Release'.
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
elseif(NOT CMAKE_BUILD_TYPE MATCHES "Release|Debug|RelWithDebInfo|MinSizeRel")
   message(FATAL_ERROR "Option CMAKE_BUILD_TYPE has invalid value \"${CMAKE_BUILD_TYPE}\"! Possible are `Release`, `Debug`, `RelWithDebInfo` and `MinSizeRel`.")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


##### DEPENDENCIES #####

# On macOS, before searching for required BISON, search Homebrew for keg-only
# versions of Bison and Flex. Xcode does not provide new enough versions.
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
   find_program(BREW brew)
   if(BREW)
      execute_process(
         COMMAND "${BREW}" --prefix bison
         RESULT_VARIABLE BREW_BISON
         OUTPUT_VARIABLE BREW_BISON_PREFIX
         OUTPUT_STRIP_TRAILING_WHITESPACE
         )
      if(BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
         message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}.")
         set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison" CACHE INTERNAL "path to the bison executable")
      endif()

      execute_process(
         COMMAND "${BREW}" --prefix flex
         RESULT_VARIABLE BREW_FLEX
         OUTPUT_VARIABLE BREW_FLEX_PREFIX
         OUTPUT_STRIP_TRAILING_WHITESPACE
         )
      if(BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
         message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}.")
         set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex" CACHE INTERNAL "path to the flex executable")
      endif()
   endif()
endif()

# Search for BISON (taking into account results above).
find_package(BISON 2.4.0 REQUIRED)

# Search for FLEX (taking into account results above).
find_package(FLEX REQUIRED)

# Search for GMP.
set(ZIMPL_HAS_GMP 0)
if(GMP)
   if(GMP STREQUAL "AUTO")
      find_package(GMP)
   else()
      find_package(GMP REQUIRED)
   endif()
   if(GMP_FOUND)
      set(ZIMPL_HAS_GMP 1)
   endif()
else()
   message(STATUS "GMP is disabled.")
endif()
if(NOT ZIMPL_HAS_GMP)
   message(WARNING "Without GMP, only the ZIMPL library will be built.")
endif()

# Support legacy FindGMP.cmake versions.
if(ZIMPL_HAS_GMP AND NOT TARGET GMP::GMP)
   if(TARGET GMP::gmp)
      message(WARNING "Found GMP version ${GMP_VERSION} has no GMP::GMP target; interfacing GMP::gmp target instead.")
      add_library(GMP::GMP INTERFACE IMPORTED)
      target_link_libraries(GMP::GMP INTERFACE GMP::gmp)
   elseif(TARGET GMP)
      message(WARNING "Found GMP version ${GMP_VERSION} has no GMP::GMP target; interfacing GMP target instead.")
      add_library(GMP::GMP INTERFACE IMPORTED)
      target_link_libraries(GMP::GMP INTERFACE GMP)
   elseif(TARGET gmp)
      message(WARNING "Found GMP version ${GMP_VERSION} has no GMP::GMP target; interfacing gmp target instead.")
      add_library(GMP::GMP INTERFACE IMPORTED)
      target_link_libraries(GMP::GMP INTERFACE gmp)
   elseif(GMP_INCLUDE_DIRS AND GMP_LIBRARIES)
      message(WARNING "Found GMP version ${GMP_VERSION} has no GMP::GMP target; using legacy variables GMP_INCLUDE_DIRS=\"${GMP_INCLUDE_DIRS}\" and GMP_LIBRARIES=\"${GMP_LIBRARIES}\" instead.")
      # GMP_LIBRARIES may be a list, so we need to create each library seperately and glue them via an INTERFACE library.
      list(LENGTH GMP_LIBRARIES num_gmp_lib)
      math(EXPR last_gmp_lib ${num_gmp_lib}-1)
      add_library(GMP_libgmp INTERFACE IMPORTED)
      foreach(id_gmp_lib RANGE ${last_gmp_lib})
         add_library(GMP::libgmp_${id_gmp_lib} UNKNOWN IMPORTED)
         list(GET GMP_LIBRARIES ${id_gmp_lib} the_gmp_lib)
         set_target_properties(GMP::libgmp_${id_gmp_lib} PROPERTIES
            IMPORTED_LOCATION "${the_gmp_lib}"
            INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIRS}")
         target_link_libraries(GMP_libgmp INTERFACE GMP::libgmp_${id_gmp_lib})
      endforeach()
      add_library(GMP::GMP INTERFACE IMPORTED)
      target_link_libraries(GMP::GMP INTERFACE GMP_libgmp)
   else()
      message(FATAL_ERROR "Found GMP version ${GMP_VERSION} has no GMP::GMP target; unable to recover!")
   endif()
endif()

# Search for ZLIB.
set(ZIMPL_HAS_ZLIB 0)
if(ZLIB)
   message(STATUS "Searching for ZLIB.")
   if(ZLIB STREQUAL "AUTO")
      find_package(ZLIB)
   else()
      find_package(ZLIB REQUIRED)
   endif()
   if(ZLIB_FOUND)
      set(ZIMPL_HAS_ZLIB 1)
   endif()
else()
   message(STATUS "ZLIB is disabled.")
endif()

# Search for Math library.
find_library(libm m)
if(NOT libm)
   set(libm "")
endif()

set(libs ${libs} ${libm})

##### TOOLS #####

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   # if changing these flags, also update GCCWARN/GXXWARN in make/make.project
   if((CMAKE_C_COMPILER_ID MATCHES "GNU") OR (CMAKE_C_COMPILER_ID MATCHES "Clang"))
      set(ADD_C_FLAGS -Wall -Wextra -Wno-unknown-pragmas -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wmissing-noreturn -Wmissing-declarations -fno-omit-frame-pointer)
   endif()

   add_compile_options(${ADD_C_FLAGS})
   if(CMAKE_C_COMPILER_ID MATCHES "GNU")
      set(ADD_GNU_FLAGS -Wsuggest-attribute=format -Wno-suggest-attribute=noreturn )
      add_compile_options(${ADD_GNU_FLAGS})
      if(CMAKE_C_COMPILER_VERSION VERSION_LESS 5)
         message(WARNING "GCC version smaller than 5, it is recommended to use version 10 or higher.")
         add_compile_options(-Wno-nonnull-compare -Wno-unused-parameter -Wno-unused-function -Wno-pragmas -Wno-clobbered -Wno-attributes)
      elseif(CMAKE_C_COMPILER_VERSION VERSION_LESS 8)
         message(WARNING "GCC version smaller than 8, it is recommended to use version 10 or higher.")
         add_compile_options(-Wno-nonnull-compare -Wno-unused-parameter -Wno-unused-function -fstack-protector-strong -Wduplicated-branches)
      elseif(CMAKE_C_COMPILER_VERSION VERSION_LESS 9)
         message(WARNING "GCC version smaller than 9, it is recommended to use version 10 or higher.")
         add_compile_options(-Wno-strict-overflow -Wno-nonnull-compare -Wno-unused-parameter -Wno-unused-function -fstack-protector-strong -Wduplicated-branches)
      else() # gcc 10 or later
         add_compile_options(-Wno-strict-overflow -Wno-unused-parameter -Wno-unused-function -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wsuggest-attribute=malloc -Wsuggest-attribute=cold -Wstrict-overflow=4 -fstack-protector-strong -Wduplicated-branches -Wno-nonnull-compare)
      endif()
   elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
      set(ADD_CLANG_FLAGS -Wno-unused-parameter -Wno-unused-function -Wno-cast-align -Wno-strict-prototypes -Wno-pragmas -Wno-attributes -Wstrict-overflow=4 -fstack-protector-strong -Wno-invalid-command-line-argument)
      set(EXTRA_CLANG_FLAGS -Weverything -Wno-reserved-id-macro -Wno-cast-qual -Wno-bad-function-cast -Wno-float-equal -Wno-unused-macros -Wno-implicit-fallthrough -Wno-switch-enum -Wno-padded -Wno-covered-switch-default -Wno-shorten-64-to-32 -Wno-pedantic -Wno-format-nonliteral -Wno-disabled-macro-expansion -Wno-static-in-inline)
      add_compile_options(${ADD_CLANG_FLAGS} ${EXTRA_CLANG_FLAGS})
      if(CMAKE_C_COMPILER_VERSION VERSION_LESS 7)
         message(WARNING "Clang version smaller than 7, it is recommended to use version 10 or higher.")
      else()
         add_compile_options(-Wno-extra-semi-stmt)
      endif()
   endif()
endif()

# Disable fused floating point contraction to enhance reproducibility across compilers and architectures.
if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
   add_compile_options(/fp:precise)
elseif(CMAKE_C_COMPILER_ID MATCHES "Intel")
   add_compile_options(-fp-model=precise -wd111,151,171,279,981,1173,1419,1684,2259)
elseif((CMAKE_C_COMPILER_ID MATCHES "GNU") OR (CMAKE_C_COMPILER_ID MATCHES "Clang"))
   add_compile_options(-ffp-contract=off)
endif()

# Filter /MD and /MDd from C and CXX flags.
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

if(MSVC)
   # look for pcre if Windows
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

##### SUBDIRECTORIES #####

add_subdirectory(src)

##### INSTALLATION #####

# Install license files.
install(
   FILES "${PROJECT_SOURCE_DIR}/LICENSE"
   DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/zimpl")

# create zimpl-config-version.cmake.
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_BINARY_DIR}/zimpl-config-version.cmake"
   VERSION ${ZIMPL_VERSION_MAJOR}.${ZIMPL_VERSION_MINOR}.${ZIMPL_VERSION_PATCH}
   COMPATIBILITY SameMajorVersion
   )

# Unless we build a package, we populate search directories for dependencies.
include(PackageConfigDependencies)

if(NOT BUILD_PACKAGE AND ZIMPL_HAS_GMP AND GMP_DIR)
   package_config_dependency_dir(ZIMPL_CONFIG_GMP_DIR GMP_FOUND GMP_DIR)
endif()
if(NOT BUILD_PACKAGE AND ZIMPL_HAS_ZLIB AND ZLIB_DIR)
   package_config_dependency_dir(ZIMPL_CONFIG_ZLIB_DIR ZLIB_FOUND ZLIB_DIR)
endif()

# Configure zimpl-config.cmake for the build-tree:
# We add the CMake module path from the sources.
set(EXTRA_CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")
configure_file("${PROJECT_SOURCE_DIR}/zimpl-config.cmake.in"
   "${CMAKE_BINARY_DIR}/zimpl-config.cmake"
   @ONLY)

# Configure zimpl-config.cmake for the installation.
# We add the current directory of the installed zimpl-config.cmake.
set(EXTRA_CMAKE_MODULE_PATH "\${CMAKE_CURRENT_LIST_DIR}")
configure_file("${PROJECT_SOURCE_DIR}/zimpl-config.cmake.in"
   "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/zimpl-config.cmake"
   @ONLY)

# Install CMake config and modules.
install(FILES
   "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/zimpl-config.cmake"
   "${CMAKE_BINARY_DIR}/zimpl-config-version.cmake"
   $<$<BOOL:${ZIMPL_HAS_GMP}>:${PROJECT_SOURCE_DIR}/cmake/Modules/FindGMP.cmake>
   DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zimpl")

# Install the zimpl-targets export set.
install(EXPORT zimpl-targets
   NAMESPACE ZIMPL::
   DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/zimpl")

# Export targets for the build-tree.
export(EXPORT zimpl-targets
   NAMESPACE ZIMPL::
   FILE "${CMAKE_BINARY_DIR}/zimpl-targets.cmake")

# If we include this directory then we mark ZIMPL as found.
set(ZIMPL_FOUND 1)

