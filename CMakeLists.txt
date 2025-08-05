cmake_minimum_required(VERSION 3.15)
project(clarinoid_desktop LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-configuration output directories
foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${cfg}" CFGU)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/bin/${cfg})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/bin/${cfg})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFGU} ${CMAKE_BINARY_DIR}/lib/${cfg})
endforeach()

# MSVC-specific settings
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/Zc:__cplusplus)  # Report correct __cplusplus value
endif()

# Global definitions
add_compile_definitions(
    CLARINOID_PLATFORM_X86
    NOMINMAX  # Prevent Windows min/max macros
)

# Source root
set(CLARINOID_SRC_ROOT "${CMAKE_SOURCE_DIR}/src")

#==============================================================================
# External Dependencies
#==============================================================================

include(FetchContent)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)

FetchContent_MakeAvailable(googletest imgui)

#==============================================================================
# Library: clarinoid_core
#==============================================================================

file(GLOB_RECURSE CLARINOID_CORE_HEADERS 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include/*.hpp"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include/*.h"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include/**/*.hpp"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include/**/*.h"
)

file(GLOB_RECURSE CLARINOID_CORE_SOURCES 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/src/*.cpp" # files directly under src
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/src/*.c"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/src/**/*.cpp" # under subdirs
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/src/**/*.c"
)

#message(STATUS "core include dir: ${CLARINOID_SRC_ROOT}/library/clarinoid_core/include/")
#message(STATUS "core src dir: ${CLARINOID_SRC_ROOT}/library/clarinoid_core/src")
# message(STATUS "Core sources: ${CLARINOID_CORE_HEADERS}")
#message(STATUS "Core sources: ${CLARINOID_CORE_SOURCES}")

add_library(clarinoid_core STATIC 
    ${CLARINOID_CORE_HEADERS}
    ${CLARINOID_CORE_SOURCES}
)

target_include_directories(clarinoid_core PUBLIC 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include"
)

target_compile_features(clarinoid_core PUBLIC cxx_std_17)

#==============================================================================
# Library: clarinoid_sim (depends on core)
#==============================================================================

file(GLOB_RECURSE CLARINOID_SIM_HEADERS 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/*.hpp"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/*.h"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/**/*.hpp"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/**/*.h"
)

file(GLOB_RECURSE CLARINOID_SIM_SOURCES 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/src/*.cpp"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/src/*.c"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/src/**/*.cpp"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/src/**/*.c"
)

#message(STATUS "Sim headers found: ${CLARINOID_SIM_HEADERS}")
#message(STATUS "Sim sources found: ${CLARINOID_SIM_SOURCES}")

add_library(clarinoid_sim STATIC 
    ${CLARINOID_SIM_HEADERS}
    ${CLARINOID_SIM_SOURCES}
)

target_include_directories(clarinoid_sim PUBLIC 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include"
)

target_compile_features(clarinoid_sim PUBLIC cxx_std_17)

target_link_libraries(clarinoid_sim PUBLIC 
    clarinoid_core
)

#==============================================================================
# Executable: simulator
#==============================================================================

add_executable(simulator WIN32
    ${CLARINOID_SRC_ROOT}/apps/ClarinoidSimulator/src/SimulatorMain.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(simulator PRIVATE 
    ${imgui_SOURCE_DIR}
)

target_compile_features(simulator PRIVATE cxx_std_17)

target_link_libraries(simulator PRIVATE 
    clarinoid_sim 
    clarinoid_core
)

#==============================================================================
# Executable: ClarinoidTests
#==============================================================================

enable_testing()

add_executable(ClarinoidTests
    ${CLARINOID_SRC_ROOT}/apps/clarinoid-core.tests/src/test_main.cpp
)

target_include_directories(ClarinoidTests PRIVATE 
    "${CLARINOID_SRC_ROOT}"
)

target_compile_features(ClarinoidTests PRIVATE cxx_std_17)

target_link_libraries(ClarinoidTests PRIVATE 
    gtest_main 
    clarinoid_core
)

include(GoogleTest)
gtest_discover_tests(ClarinoidTests)

#==============================================================================
# IDE Visibility Project (optional - for complete library structure)
#==============================================================================

file(GLOB_RECURSE CLARINOID_LIBRARY_ALL_FILES 
    "${CLARINOID_SRC_ROOT}/library/**/*"
)

# Filter out directories
set(CLARINOID_LIBRARY_FILES)
foreach(file ${CLARINOID_LIBRARY_ALL_FILES})
    if(NOT IS_DIRECTORY ${file})
        list(APPEND CLARINOID_LIBRARY_FILES ${file})
    endif()
endforeach()

add_library(ClarinoidLibraryFiles STATIC 
    ${CLARINOID_LIBRARY_FILES}
)

# Preserve directory structure in Visual Studio
foreach(file ${CLARINOID_LIBRARY_FILES})
    file(RELATIVE_PATH rel_path "${CLARINOID_SRC_ROOT}/library" ${file})
    get_filename_component(dir_path ${rel_path} DIRECTORY)
    string(REPLACE "/" "\\" group_name ${dir_path})
    source_group(${group_name} FILES ${file})
endforeach()

# Exclude from build (IDE visibility only)
set_target_properties(ClarinoidLibraryFiles PROPERTIES 
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE
)

target_compile_features(ClarinoidLibraryFiles PRIVATE cxx_std_17)

target_include_directories(ClarinoidLibraryFiles PRIVATE 
    "${CLARINOID_SRC_ROOT}"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include"
)
