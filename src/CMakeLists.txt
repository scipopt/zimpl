# Overall structure:
#  
# Configure build-dependent header file.
# Define common compilation settings.
# Define targets along with properties, dependencies, includes, linked libraries, definitions.
# Define target aliases.
# Add sanitizers, etc.
# Install headers.
# Install targets.
# Generate export header file.

# Run Bison and Flex.
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/zimpl")
bison_target(MMLPARSE2
   zimpl/mmlparse2.y
   "${PROJECT_BINARY_DIR}/zimpl/mmlparse2.c"
   COMPILE_FLAGS -t
   )
flex_target(MMLSCAN
   zimpl/mmlscan.l
   "${PROJECT_BINARY_DIR}/zimpl/mmlscan.c"
   )

# Note that the scope of the following settings is local.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# ZIMPL library: Note that we always build a static library as otherwise
# the required glueing code is hard to import from another shared library
# (on Mac) or DLL (on Windows).
add_library(libzimpl STATIC
   zimpl/blkmem.c
   zimpl/bound.c
   zimpl/code.c
   zimpl/conname.c
   zimpl/define.c
   zimpl/elem.c
   zimpl/entry.c
   zimpl/hash.c
   zimpl/heap.c
   zimpl/idxset.c
   zimpl/inst.c
   zimpl/iread.c
   zimpl/list.c
   zimpl/load.c
   zimpl/local.c
   zimpl/metaio.c
   zimpl/mono.c
   zimpl/mshell.c
   zimpl/prog.c
   zimpl/random.c
   zimpl/rdefpar.c
   zimpl/source.c
   zimpl/setempty.c
   zimpl/setpseudo.c
   zimpl/setlist.c
   zimpl/setrange.c
   zimpl/setprod.c
   zimpl/setmulti.c
   zimpl/set4.c
   zimpl/stmt.c
   zimpl/stkchk.c
   zimpl/strstore2.c
   zimpl/symbol.c
   zimpl/term2.c
   zimpl/tuple.c
   zimpl/vinst.c
   zimpl/zimpllib.c
   $<$<BOOL:${ZIMPL_HAS_GMP}>:zimpl/gmpmisc.c>
   $<$<BOOL:${ZIMPL_HAS_GMP}>:zimpl/numbgmp.c>
   $<$<NOT:$<BOOL:${ZIMPL_HAS_GMP}>>:zimpl/numbdbl.c>
   $<$<BOOL:${MSVC}>:WIN/getopt.c>
   ${BISON_MMLPARSE2_OUTPUTS}
   ${FLEX_MMLSCAN_OUTPUTS}
   )
target_include_directories(libzimpl BEFORE
   PUBLIC
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
      "$<$<BOOL:${MSVC}>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/WIN>>"
      "$<INSTALL_INTERFACE:include>"
      "$<$<BOOL:${MSVC}>:$<INSTALL_INTERFACE:include/WIN>>"
   )
target_link_libraries(libzimpl
   PUBLIC
      $<$<BOOL:${ZIMPL_HAS_GMP}>:GMP::GMP>
      $<$<BOOL:${ZIMPL_HAS_ZLIB}>:ZLIB::ZLIB>
      ${libs}
   )
target_compile_definitions(libzimpl
   PRIVATE
      $<$<NOT:$<BOOL:${ZIMPL_HAS_ZLIB}>>:WITHOUT_ZLIB>
      VERSION=\"${ZIMPL_VERSION_MAJOR}.${ZIMPL_VERSION_MINOR}.${ZIMPL_VERSION_PATCH}\"
   )
set_target_properties(libzimpl PROPERTIES
   PREFIX ""
#   TODO: once we annotated functions with ZIMPL_EXPORT and generate `zimpl_export.h`:
#   C_VISIBILITY_PRESET hidden
#   VISIBILITY_INLINES_HIDDEN 1
   )

if(ZIMPL_HAS_GMP)
   # ZIMPL executable.
   set(zimpl_executable "zimpl-executable")
   add_executable(zimpl-executable
      zimpl/zimpl.c
      zimpl/xlpglue.c
      zimpl/zlpglue.c
      zimpl/ratlpstore.c
      zimpl/ratlpfwrite.c
      zimpl/ratmpswrite.c
      zimpl/ratmstwrite.c
      zimpl/ratopbwrite.c
      zimpl/ratordwrite.c
      zimpl/ratqbowrite.c
      zimpl/ratpresolve.c
      )
   target_link_libraries(zimpl-executable libzimpl)
   target_compile_definitions(zimpl-executable
      PRIVATE
         VERSION=\"${ZIMPL_VERSION_MAJOR}.${ZIMPL_VERSION_MINOR}.${ZIMPL_VERSION_PATCH}\")
   set_target_properties(zimpl-executable PROPERTIES
      OUTPUT_NAME zimpl
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
      )
endif()

# Create aliases internally in case ZIMPL is included via `add_subdirectory`.
add_library(ZIMPL::ZIMPL ALIAS libzimpl)
add_library(ZIMPL::libzimpl ALIAS libzimpl)
if(zimpl_executable)
   add_executable(ZIMPL::zimpl-executable ALIAS zimpl-executable)
endif()

# Add sanitizers.
if(SANITIZE)
   find_package(Sanitizers)
   add_sanitizers(libzimpl)
   if(zimpl_executable)
      add_sanitizers(zimpl-executable)
   endif()
endif()

# Install public header files.
foreach(header_file
   zimpl/attribute.h
   zimpl/blkmem.h
   zimpl/bound.h
   zimpl/code.h
   zimpl/conname.h
   zimpl/define.h
   zimpl/elem.h
   zimpl/entry.h
   zimpl/gmpmisc.h
   zimpl/hash.h
   zimpl/heap.h
   zimpl/idxset.h
   zimpl/inst.h
   zimpl/lint.h
   zimpl/list.h
   zimpl/local.h
   zimpl/metaio.h
   zimpl/mme.h
   zimpl/mono.h
   zimpl/mshell.h
   zimpl/numb.h
   zimpl/prog.h
   zimpl/random.h
   zimpl/ratlp.h
   zimpl/ratlpstore.h
   zimpl/ratlptypes.h
   zimpl/rdefpar.h
   zimpl/set4.h
   zimpl/set.h
   zimpl/stkchk.h
   zimpl/stmt.h
   zimpl/strstore.h
   zimpl/symbol.h
   zimpl/term.h
   zimpl/tuple.h
   zimpl/xlpglue.h
   zimpl/zimpllib.h
   zimpl/zlpglue.h
   )
   # Extract directory part.
   get_filename_component(directory "${header_file}" DIRECTORY)
   # Install the header file.
   install(
      FILES "${PROJECT_SOURCE_DIR}/src/${header_file}"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${directory}")
endforeach()

# TODO: Check if we need to install the Bison-generated headers. They are not included by any ZIMPL header.
# Install Bison-generated headers. 
#install(
#   FILES ${BISON_MMLPARSE2_OUTPUT_HEADER}
#   DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/zimpl")

# Install Windows-specific header files.
if(MSVC)
   install(
      FILES WIN/unistd.h WIN/getopt.h
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/WIN")
endif()
if(MSVC AND zimpl_executable)
   install(
      FILES $<TARGET_PDB_FILE:zimpl-executable>
      DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL)
endif()

# Install targets, and record them in zimpl-targets export set.
install(TARGETS ${zimpl_executable} libzimpl
   EXPORT zimpl-targets
   LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
   ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
   RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
   INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
   )

