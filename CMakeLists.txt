cmake_minimum_required(VERSION 3.20.0)

project(compiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

find_package(LLVM 18.1.8 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_definitions(-DGTEST_HAS_CXXABI_H_=0) # dont touch this

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/tests)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(compiler main.cpp ${SOURCES})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(compiler ${llvm_libs})

enable_testing()

find_package(GTest REQUIRED)

add_executable(lexer_tests ../tests/lexer_tests.cpp)
add_executable(parser_tests ../tests/parser_tests.cpp)

target_link_libraries(lexer_tests GTest::gtest GTest::gtest_main)
target_link_libraries(parser_tests GTest::gtest GTest::gtest_main)

add_test(AllTestsInFoo lexer_tests)
add_test(AllTestsInFoo parser_tests)