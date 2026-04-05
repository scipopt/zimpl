# Zimpl Agent Guidelines

This file provides guidance for agentic coding agents working in the Zimpl repository.

## Build, Lint, and Test Commands

### Building the Project
The following parameters are available:
- OPT=dbg for debugging builds (default)
- OPT=opt for optimized builds
- COMP=gnu for the gnu gcc compiler (default)
- COMP=clang for the LLVM clang compiler
- Standard build: `make`
- Debug build: `make OPT=dbg`
- Optimized build: `make OPT=opt`
- Clean build: `make clean`
- Generate dependencies: `make depend`
- Generate documentation: `make doc`
- Generate Doxygen docs: `make doxygen`
Note: make is equal to `make OPT=dbg`. To clean the optimized build: `make OPT=opt clean`
To clean the optimized clang build it is `make OPT=opt COMP=clang clean`

### Linting
- Run Cppcheck: `make cppcheck`

### Testing
- Run all regression tests: `make check`
- Run tests with valgrind: `make valgrind` (only on x86_64)
- Run coverage analysis: first `make OPT=gcov` then  `make coverage`
- Run static analysis: `make analyze`

### Other Commands
- Verbose build: `make VERBOSE=true`
- Analyzer: `make analyze` uses clang scan-build
- Parallel builds: Use `-j` flag with make (e.g., `make -j4`)

## Code Style Guidelines

### Formatting
- Indentation: 3 spaces (no tabs)
- Line length: Aim for ≤ 80 characters, but not strictly enforced
- Braces: opening brace on line after the statement
- File headers: Standard copyright/header comment block at top of each file
- Comments: Use /* */ for block comments, // for single-line comments when needed
- Declaration style: Mixed declarations and code allowed (C99 feature)
- Trailing whitespace: Avoid trailing whitespace on lines
- Empty lines: Use empty lines to separate logical sections in functions
- Use East style const

### Imports/Includes
- Order: System includes, then project includes
- Project includes: Alphabetical order within the "zimpl/" directory
- Style: `#include <stdio.h>` for system, `#include "zimpl/header.h"` for project
- Guard against multiple inclusion: Use include guards in header files (e.g., `#ifndef _HEADER_H_`)
- Include path: Project includes use `-I$(SRCDIR)/..` flag in Makefile
- Conditional includes: Use #ifdef for platform-specific includes when necessary

### Types
- Boolean: Use `bool` from stdbool.h
- Integer sizes: Use standard C types (int, long, etc.) or defined types from project
- Pointers: Use `NULL` for null pointers
- Custom types: Defined in relevant header files (e.g., in zimpl/ directory)
- Memory-specific types: Use project's blob memory allocator (blkmem) for allocations
- Precision types: Use project-defined types for numerical precision when needed
- Size types: Use size_t for sizes and counts, ssize_t when signed values are needed

### Naming Conventions
- Functions: snake_case (e.g., `function_name`)
- Variables: snake_case (e.g., `variable_name`)
- Constants: UPPER_SNAKE_CASE (e.g., `CONSTANT_NAME`)
- Macros: UPPER_SNAKE_CASE (e.g., `#define MACRO_NAME`)
- Structs/tags: snake_case with _t suffix for typedefs (e.g., `typedef struct {...} struct_name_t;`)
- Enumeration values: UPPER_SNAKE_CASE
- File-local static variables: Often prefixed with `static` and use snake_case
- Global variables: Declared extern in headers, defined in source files
- Function parameters: Use descriptive names in snake_case
- Loop counters: Typically i, j, k for simple loops, descriptive names for complex loops

### Error Handling
- Check return values from system calls and library functions
- Use assertions for preconditions and invariants (`assert(condition)`)
- Propagate errors up the call chain when appropriate
- Use errno for system call errors when applicable
- Exit with appropriate error codes (typically non-zero for failure)
- Error messages: Often printed to stderr with fprintf
- Error recovery: Clean up allocated resources before exiting on error
- Error propagation: Return error codes up the call chain when exceptions aren't used

### Specific Patterns Observed
- File headers follow a standard format with copyright and description
- Global variables are declared extern in headers and defined in source files
- Static functions are used for file-local scope
- Function prototypes in headers, definitions in source files
- Use of inline functions where appropriate (defined in headers)
- Memory management: Uses project-specific allocators (blkmem) rather than direct malloc/free
- String handling: Custom string storage mechanism (strstore) rather than direct string manipulation
- Error checking: Liberal use of assert() for debugging conditions
- Command-line parsing: Standard getopt() pattern with option strings
- Debug printing: Often guarded by debug flags or verbose levels
- Constants: Defined using enum or #define for related values, const for single values

### Language Standard
- Primarily C99 (uses bool from stdbool.h, mixed declarations and code)
- Some GNU extensions used conditionally (visible in GCCWARN flags)
- No C++ features (this is a pure C project)
- Variadic macros used in some headers for debugging
- Designated initializers used in some struct initializations

### Generated Files
- Lexer: Generated from mmlscan.l by flex → mmlscan.c
- Parser: Generated from mmlparse2.y by bison → mmlparse2.c
- Header: mme.h is shared between lexer and parser
- Do not modify generated files directly; modify the source .l and .y files instead
- Regenerate generated files when .l or .y files change

### Windows-Specific Code
- Located in src/WIN/ directory
- Contains Windows-specific implementations (getopt, unistd.h)
- Requires specific defines: WIN32 or WIN64 and INLINE=
- For Windows builds with Visual Studio, add src/WIN/ files to compilation
- Windows code follows same style guidelines but may need platform-specific adaptations

### Additional Notes
- The project uses custom lexer (flex) and parser (bison) generated from .l and .y files
- Makefile supports various architectures and compilers through make/make.* files
- Library uses GNU Multiple Precision Arithmetic Library (GMP) and zlib
- Debug builds enable additional checking (NDEBUG and NO_MSHELL not defined)
- String functions: Prefer project's string functions over standard library when available
- Integer operations: Check for overflow when doing arithmetic that could exceed limits
- Floating point: Be aware of precision issues when comparing floating point values
- This project uses mostly GMP for rational arithmetic