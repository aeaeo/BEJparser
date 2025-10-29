# About
Binary-encoded JSON (BEJ) parser to UTF-8 json format following the DMTF DSP0218 specification.

# Build
This following reference build is made on GNU/Linux Debian 13 Trixie.

Next packages are required: libgtest-dev googletest doxygen cmake
CMake is at least ver. 3.3.0

In terminal, go to the project root dir then execute the next following commands:
Let the clang be our compiler
$ CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -S . -B ./build/ -DBUILD_TESTS=ON -DBUILD_DOC=ON
-- The C compiler identification is Clang 19.1.7
-- The CXX compiler identification is Clang 19.1.7
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/clang - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Unit tests enabled with Google Test
-- 	Run tests with: make test
-- 	Or directly: ./BEJtests
-- Doxygen found. Run 'make doc' to generate documentation.
-- 
-- ========================================
-- BEJparser Configuration Summary
-- ========================================
--   Build type:       
--   C Compiler:       /usr/bin/clang
--   C Flags:           -Wall -Wextra
--   Install prefix:   /usr/local
--   Build tests:      ON
--   GTest found:      YES
--   Doxygen found:      YES
-- ========================================
-- 
-- Configuring done (1.0s)
-- Generating done (0.0s)
-- Build files have been written to: .../BEJparser/build

$ cd ./build
$ make
[ 16%] Building C object CMakeFiles/BEJparser.dir/src/main.c.o
[ 33%] Building C object CMakeFiles/BEJparser.dir/src/bej.c.o
[ 50%] Linking C executable BEJparser
[ 50%] Built target BEJparser
[ 66%] Building CXX object CMakeFiles/BEJtests.dir/unit_tests/test_bej.cpp.o
[ 83%] Building C object CMakeFiles/BEJtests.dir/src/bej.c.o
[100%] Linking CXX executable BEJtests
[100%] Built target BEJtests


# Build