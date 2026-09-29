cmake_minimum_required(VERSION 3.16)

project(typeAssist
    VERSION 0.1.0
    DESCRIPTION "System-wide typing assistant"
    LANGUAGES CXX
)

# ---- C++ settings -----------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Core engine library ----------------------------------------------------
# The engine knows nothing about Windows or AI. Later, the Windows hook and
# the AI layer will link against this library.
add_library(typeassist_core
    src/TextEngine.cpp
)

target_include_directories(typeassist_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Warnings
if(MSVC)
    target_compile_options(typeassist_core PRIVATE /W4)
else()
    target_compile_options(typeassist_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- Demo executable --------------------------------------------------------
add_executable(typeassist_demo
    src/main.cpp
)

target_link_libraries(typeassist_demo PRIVATE typeassist_core)

# ---- Tests (enabled in a later step) ----------------------------------------
# enable_testing()
# add_subdirectory(tests)