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
    # Force MSVC to report the correct __cplusplus value
    add_compile_options(/Zc:__cplusplus)
endif()

# Define source root for easier path management
set(CLARINOID_SRC_ROOT "${CMAKE_SOURCE_DIR}/src")

include_directories("${CLARINOID_SRC_ROOT}")
include_directories("${CLARINOID_SRC_ROOT}/library/clarinoid_core/include")
include_directories("${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include")
add_compile_definitions(CLARINOID_PLATFORM_X86) # Define platform for compilation
add_compile_definitions(NOMINMAX) # Prevent windows min/max macros from interfering with everything

# GoogleTest and ImGui
include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2 # or latest
)
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG master # or a specific release tag
)
FetchContent_MakeAvailable(googletest imgui)

add_executable(simulator WIN32
  ${CLARINOID_SRC_ROOT}/apps/ClarinoidSimulator/src/SimulatorMain.cpp
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp)
target_include_directories(simulator PRIVATE ${imgui_SOURCE_DIR})
target_compile_features(simulator PRIVATE cxx_std_17)
# include_directories("${CLARINOID_SRC_ROOT}/apps/ClarinoidSimulator/include")


# ClarinoidTests project
enable_testing()
add_executable(ClarinoidTests
  ${CLARINOID_SRC_ROOT}/apps/clarinoid-core.tests/src/test_main.cpp)
target_include_directories(ClarinoidTests PRIVATE "${CLARINOID_SRC_ROOT}")
target_compile_features(ClarinoidTests PRIVATE cxx_std_17)
# include_directories("${CLARINOID_SRC_ROOT}/apps/clarinoid-core.tests/include")
target_link_libraries(ClarinoidTests gtest_main)

include(GoogleTest)
gtest_discover_tests(ClarinoidTests)

# Library visibility project - includes all library files for Visual Studio Solution Explorer
file(GLOB_RECURSE CLARINOID_LIBRARY_ALL_FILES 
    "${CLARINOID_SRC_ROOT}/library/**/*"
)

# Filter out directories (keep only files)
set(CLARINOID_LIBRARY_FILES)
foreach(file ${CLARINOID_LIBRARY_ALL_FILES})
    if(NOT IS_DIRECTORY ${file})
        list(APPEND CLARINOID_LIBRARY_FILES ${file})
    endif()
endforeach()

add_library(ClarinoidLibraryFiles STATIC 
    ${CLARINOID_LIBRARY_FILES}
)

# Preserve the directory structure in Visual Studio Solution Explorer
foreach(file ${CLARINOID_LIBRARY_FILES})
    # Get the relative path from the library root
    file(RELATIVE_PATH rel_path "${CLARINOID_SRC_ROOT}/library" ${file})
    get_filename_component(dir_path ${rel_path} DIRECTORY)
    
    # Convert forward slashes to backslashes for Windows-style folder structure
    string(REPLACE "/" "\\" group_name ${dir_path})
    
    # Create source groups that preserve the directory structure
    source_group(${group_name} FILES ${file})
endforeach()

# Exclude this library from the build by default (it's just for IDE visibility)
set_target_properties(ClarinoidLibraryFiles PROPERTIES 
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE
)

# Set C++17 for the library files project
target_compile_features(ClarinoidLibraryFiles PRIVATE cxx_std_17)

# Set up proper include directories for the library
target_include_directories(ClarinoidLibraryFiles PRIVATE 
    "${CLARINOID_SRC_ROOT}"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include"
)
