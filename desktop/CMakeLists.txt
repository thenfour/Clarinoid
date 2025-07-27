
cmake_minimum_required(VERSION 3.15)
project(clarinoid_desktop LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Set output directories for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
  string(TOUPPER "${cfg}" CFGU)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/bin/${cfg})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/bin/${cfg})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/lib/${cfg})
endforeach()

# Set MSVC runtime library if using MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Add global include directory for ../src
include_directories("${CMAKE_SOURCE_DIR}/../src")

add_executable(simulator
  simulator/main.cpp
)
target_compile_definitions(simulator PRIVATE CLARINOID_PLATFORM_X86)


# GoogleTest
include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2) # or latest
FetchContent_MakeAvailable(googletest)
enable_testing()

# ClarinoidTests project
add_executable(ClarinoidTests
  tests/main.cpp
)
target_compile_definitions(ClarinoidTests PRIVATE CLARINOID_PLATFORM_X86)
target_include_directories(ClarinoidTests PRIVATE "${CMAKE_SOURCE_DIR}/../src")
set_target_properties(ClarinoidTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)
target_link_libraries(ClarinoidTests gtest_main)

include(GoogleTest)
gtest_discover_tests(ClarinoidTests)
