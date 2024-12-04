cmake_minimum_required(VERSION 3.20.0)

project(compiler VERSION 1.0)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall") # needed by binaryen

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(Parser ${PROJECT_SOURCE_DIR}/src/parser.y ${PROJECT_SOURCE_DIR}/src/parser2.cpp DEFINES_FILE ${PROJECT_SOURCE_DIR}/include/parser2.hpp)
FLEX_TARGET(Lexer ${PROJECT_SOURCE_DIR}/src/lexer.l ${PROJECT_SOURCE_DIR}/src/lexer2.cpp)

include_directories("/usr/include/") # needed by binaryen, includes binaryen-c.h
link_directories("/usr/lib/x86_64-linux-gnu/") # needed by binaryen, includes libbinaryen.so

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/tests)

file(GLOB_RECURSE SOURCES "src/*.cpp")

set(RUN_WASM_FILE_LOCATION "/workspaces/bird-lang/tests/helpers/run_wasm.js")
add_compile_definitions(FILE_LOCATION="${FILE_LOCATION}")


add_executable(compiler main.cpp ${SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})

option(DEBUG "Build with debug flags" OFF)

if (DEBUG)
  target_compile_definitions(compiler PRIVATE DEBUG)
endif()

target_compile_definitions(compiler PRIVATE RUN_WASM_FILE_LOCATION)

target_link_libraries(compiler binaryen)

option(TESTS "Build the tests" OFF)

if (TESTS) 
  message("Building tests")
  enable_testing()

  find_package(GTest REQUIRED)

  file(GLOB_RECURSE TEST_SOURCES tests/*_test.cpp)

  # For each test file, create a separate test executable
  foreach(test_source ${TEST_SOURCES})
      get_filename_component(test_name ${test_source} NAME_WE)
      add_executable(${test_name} ${test_source})
      target_link_libraries(${test_name} GTest::gtest GTest::gtest_main)
      target_link_libraries(${test_name} binaryen)
      add_test(NAME ${test_name} COMMAND ${test_name})

    target_compile_definitions(${test_name} PRIVATE RUN_WASM_FILE_LOCATION="${RUN_WASM_FILE_LOCATION}")
  endforeach()

endif()
