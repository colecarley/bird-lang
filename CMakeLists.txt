cmake_minimum_required(VERSION 3.20.0)

project(compiler VERSION 1.0)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

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
llvm_map_components_to_libnames(llvm_libs ${LLVM_TARGETS_TO_BUILD} support core irreader)

foreach(target ${LLVM_TARGETS_TO_BUILD})
  list(APPEND targets "LLVM${target}CodeGen")
endforeach()

target_link_libraries(compiler ${llvm_libs} ${targets})

enable_testing()

find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES tests/*_test.cpp)

# For each test file, create a separate test executable
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} GTest::gtest GTest::gtest_main)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()