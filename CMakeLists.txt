cmake_minimum_required(VERSION 3.16)

project(OpenMachineLab
    VERSION 0.1.0
    DESCRIPTION "A generic runtime model for industrial equipment software"
    LANGUAGES CXX)

# Register CTest targets (tests/lifecycle_test).
enable_testing()

# --- language & global settings ------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# Emit compile_commands.json so clangd (and other tooling) sees the exact
# flags of every translation unit. Required for the project's clangd setup.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# std::thread is used by the example's operator console.
find_package(Threads REQUIRED)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4 /permissive-)
endif()

# --- the runtime library -------------------------------------------------
# Phase 1 layout: one folder per concept. There is no "core" umbrella on
# purpose - the model is small enough that one library covers it.
add_library(OpenMachineLab STATIC
    config/DeviceConfig.cpp
    history/History.cpp
    host/Host.cpp
    log/Logger.cpp
    state/MachineState.cpp
    workflow/RecipeSpec.cpp
    workflow/Workflow.cpp
    machine/Machine.cpp
)

# Headers sit next to their sources under <concept>/. Exposing the project
# root lets code write #include "machine/Machine.h" uniformly.
target_include_directories(OpenMachineLab PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# --- examples ------------------------------------------------------------
add_subdirectory(examples/minimal_machine)
add_subdirectory(examples/pick_and_place)
add_subdirectory(examples/host_demo)

# --- tests ---------------------------------------------------------------
add_subdirectory(tests)
