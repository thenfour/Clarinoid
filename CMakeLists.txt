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

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/src/desktop/include")

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
enable_testing()

add_executable(simulator WIN32
  ${CMAKE_SOURCE_DIR}/src/desktop/simulator/SimulatorMain.cpp
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
   "src/clarinoid/x86/ArduinoEmu.cpp")
target_include_directories(simulator PRIVATE ${imgui_SOURCE_DIR})
target_compile_definitions(simulator PRIVATE CLARINOID_PLATFORM_X86)
target_compile_definitions(simulator PRIVATE NOMINMAX) # Prevent windows min/max macros from interfering with everything.


# ClarinoidTests project
add_executable(ClarinoidTests
  ${CMAKE_SOURCE_DIR}/src/desktop/tests/TestsMain.cpp
 "src/clarinoid/x86/ArduinoEmu.cpp")
target_compile_definitions(ClarinoidTests PRIVATE CLARINOID_PLATFORM_X86)
target_include_directories(ClarinoidTests PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_definitions(ClarinoidTests PRIVATE NOMINMAX) # Prevent windows min/max macros from interfering with everything.
set_target_properties(ClarinoidTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)
target_link_libraries(ClarinoidTests gtest_main)

include(GoogleTest)
gtest_discover_tests(ClarinoidTests)
