cmake_minimum_required(VERSION 3.10)
project(Oscilloscope CXX)

set(CMAKE_CXX_STANDARD 17) # Use C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-O3 -march=native)
endif()

# Set CMAKE_BUILD_TYPE to Debug if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "CMAKE_BUILD_TYPE not specified, defaulting to Debug")
endif()

set(RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIR})
set(DESTINATION_PATH ${RUNTIME_OUTPUT_DIR}/$<CONFIG>)

add_subdirectory(extern/SDL3)

# Specify source files
set(SOURCES
    Main.cpp
    Oscilloscope.h
)


set(IMGUI_SOURCES
    extern/ImGui/imgui.cpp
    extern/ImGui/imgui_draw.cpp
    extern/ImGui/imgui_tables.cpp
    extern/ImGui/imgui_widgets.cpp
    extern/ImGui/imgui_demo.cpp 
    extern/ImGui/backends/imgui_impl_sdl3.cpp
    extern/ImGui/backends/imgui_impl_sdlrenderer3.cpp
)


add_executable(Oscilloscope ${SOURCES} ${IMGUI_SOURCES})

target_include_directories(Oscilloscope PRIVATE 
    ${CMAKE_SOURCE_DIR}/extern/SDL3/include/ 
    ${CMAKE_SOURCE_DIR}/extern/ImGui/ 
    ${CMAKE_SOURCE_DIR}/extern/ImGui/backends/)





# Link SDL3 
target_link_libraries(Oscilloscope SDL3::SDL3)

set_target_properties(Oscilloscope PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIR}
)



