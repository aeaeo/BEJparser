cmake_minimum_required(VERSION 3.30)
project(BEJparser C CXX)

# using latest
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -NDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

set(SOURCES src/main.c src/bej.c)
set(HEADERS src/bej.h)

include_directories(include)
add_executable(BEJparser ${SOURCES} ${HEADERS})
enable_testing()

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    # find first
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        set(TEST_SOURCES unit_tests/test_bej.cpp src/bej.c)
        
        add_executable(BEJtests ${TEST_SOURCES})
        # linking with gtest
        target_link_libraries(BEJtests GTest::GTest GTest::Main)
        target_include_directories(BEJtests PRIVATE include)
        
        # Set C flags for bej.c when compiled in test
        target_compile_definitions(BEJtests PRIVATE DEBUG)
        
        add_test(NAME bej_unit_tests COMMAND BEJtests)
        # verbose test output
        add_test(NAME bej_unit_tests_verbose COMMAND BEJtests --gtest_color=yes --gtest_print_time=1)
        
        message(STATUS "Unit tests enabled with Google Test")
        message(STATUS "\tRun tests with: make test")
        message(STATUS "\tOr directly: ./BEJtests")
    else()
        message(STATUS "Google Test not found. Unit tests disabled.")
        message(STATUS "To enable tests, install Google Test packages")
        message(STATUS "\tUbuntu/Debian: sudo apt-get install libgtest-dev googletest")
        message(STATUS "\tmacOS: brew install googletest")
    endif()
endif()

# doxygen documentation
option(BUILD_DOC "Build documentation" ON)
if(BUILD_DOC)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        set(DOXYGEN_PROJECT_NAME "BEJparser")
        set(DOXYGEN_PROJECT_BRIEF "A binary-encoded JSON (BEJ) parser following the DMTF DSP0218 specification")
        set(DOXYGEN_PROJECT_VERSION "1.0.0")
        
        # Check if Doxyfile.in exists
        if(EXISTS ${DOXYGEN_IN})
            configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
            
            add_custom_target(doc
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM
            )
            
            message(STATUS "Doxygen found. Run 'make doc' to generate documentation.")
        else()
            message(STATUS "Doxyfile.in not found at ${DOXYGEN_IN}")
        endif()
    else()
        message(STATUS "Doxygen not found. Documentation generation disabled.")
    endif()
endif()

# Installation
install(TARGETS BEJparser DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include)

# Print summary
message(STATUS "")
message(STATUS "========================================")
message(STATUS "BEJparser Configuration Summary")
message(STATUS "========================================")
message(STATUS "  Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler:       ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags:          ${CMAKE_C_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "  Debug flags:      ${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "  Release flags:    ${CMAKE_C_FLAGS_RELEASE}")
endif()
message(STATUS "  Install prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests:      ${BUILD_TESTS}")
if(BUILD_TESTS AND GTest_FOUND)
    message(STATUS "  GTest found:      YES")
else()
    message(STATUS "  GTest found:      NO")
endif()
if(BUILD_DOC AND DOXYGEN_FOUND)
    message(STATUS "  Doxygen found:      YES")
else()
    message(STATUS "  Doxygen found:      NO")
endif()
message(STATUS "========================================")
message(STATUS "")
