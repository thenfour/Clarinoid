cmake_minimum_required(VERSION 3.15)
project(clarinoid_desktop LANGUAGES CXX)

#==============================================================================
# Define a function to add files with preserved directory structure
function(add_files_with_structure target_name glob_pattern base_path_for_relative)
    file(GLOB_RECURSE found_files ${glob_pattern})
    
    # message(STATUS "Target: ${target_name}")
    # message(STATUS "Pattern: ${glob_pattern}")
    # message(STATUS "Found files: ${found_files}")
    
    # Filter out directories
    set(filtered_files)
    foreach(file ${found_files})
        if(NOT IS_DIRECTORY ${file})
            list(APPEND filtered_files ${file})
        endif()
    endforeach()
    
    # message(STATUS "Filtered files: ${filtered_files}")
    
    # Add files to target
    target_sources(${target_name} PRIVATE ${filtered_files})
    
    # Preserve directory structure
    foreach(file ${filtered_files})
        file(RELATIVE_PATH rel_path ${base_path_for_relative} ${file})
        get_filename_component(dir_path ${rel_path} DIRECTORY)
        string(REPLACE "/" "\\" group_name ${dir_path})
        source_group(${group_name} FILES ${file})
    endforeach()
endfunction()


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

add_library(clarinoid_core STATIC)

# Add files with preserved directory structure
add_files_with_structure(clarinoid_core 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include/clarinoid/core/**/*"
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include/clarinoid/")

add_files_with_structure(clarinoid_core 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/src/**/*" 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core")

add_files_with_structure(clarinoid_core 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/src/*" 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core")

target_include_directories(clarinoid_core PUBLIC 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_core/include"
)

target_compile_features(clarinoid_core PUBLIC cxx_std_17)

#==============================================================================
# Library: clarinoid_sim (depends on core)
#==============================================================================

add_library(clarinoid_sim STATIC)

# Add files with preserved directory structure
add_files_with_structure(clarinoid_sim 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/clarinoid/sim/**/*" 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/clarinoid/")

    add_files_with_structure(clarinoid_sim 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/clarinoid/sim/*" 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/include/clarinoid/")

add_files_with_structure(clarinoid_sim 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/src/**/*" 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim")

add_files_with_structure(clarinoid_sim 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim/src/*" 
    "${CLARINOID_SRC_ROOT}/library/clarinoid_sim")

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

add_executable(ClarinoidSimulator WIN32
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

source_group("imgui_src" FILES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

# Add simulator app files with preserved directory structure
add_files_with_structure(ClarinoidSimulator 
    "${CLARINOID_SRC_ROOT}/apps/ClarinoidSimulator/**/*" 
    "${CLARINOID_SRC_ROOT}/apps/ClarinoidSimulator")

target_include_directories(ClarinoidSimulator PRIVATE 
    ${imgui_SOURCE_DIR}
    "${CLARINOID_SRC_ROOT}/apps/ClarinoidSimulator/include" 
)

target_compile_features(ClarinoidSimulator PRIVATE cxx_std_17)

target_link_libraries(ClarinoidSimulator PRIVATE 
    clarinoid_sim 
    clarinoid_core
)

#==============================================================================
# Executable: ClarinoidTests (for C++17)
#==============================================================================

enable_testing()

add_executable(ClarinoidTestsCpp17)

# Add test files with preserved directory structure
add_files_with_structure(ClarinoidTestsCpp17 
    "${CLARINOID_SRC_ROOT}/apps/clarinoid-core.tests/**/*" 
    "${CLARINOID_SRC_ROOT}/apps/clarinoid-core.tests")

target_include_directories(ClarinoidTestsCpp17 PRIVATE 
    "${CLARINOID_SRC_ROOT}"
)

target_compile_features(ClarinoidTestsCpp17 PRIVATE cxx_std_17)

target_link_libraries(ClarinoidTestsCpp17 PRIVATE 
    gtest_main 
    clarinoid_core
)

include(GoogleTest)
gtest_discover_tests(ClarinoidTestsCpp17)


#==============================================================================
# Executable: ClarinoidTests (for C++20)
#==============================================================================
add_executable(ClarinoidTestsCpp20)

add_files_with_structure(ClarinoidTestsCpp20 
    "${CLARINOID_SRC_ROOT}/apps/clarinoid-core.tests/**/*" 
    "${CLARINOID_SRC_ROOT}/apps/clarinoid-core.tests")

target_include_directories(ClarinoidTestsCpp20 PRIVATE 
    "${CLARINOID_SRC_ROOT}"
)

# ‑- force C++20 only for this target
set_target_properties(ClarinoidTestsCpp20 PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

target_link_libraries(ClarinoidTestsCpp20 PRIVATE 
    gtest_main 
    clarinoid_core
)

gtest_discover_tests(ClarinoidTestsCpp20)
