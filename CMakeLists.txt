cmake_minimum_required(VERSION 3.13)

project(chip8_vm LANGUAGES CXX VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE on)

# Require ISO C++17 support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable LTO
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Enable Warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Link with SFML
link_libraries(sfml-graphics sfml-system sfml-window)

# Define executables
add_executable(chip8_vm src/main.cpp
                        src/cpu.cpp
                        src/graphics.cpp
                        src/input.cpp
                        src/instruction.cpp
                        src/rom.cpp
                        src/timer.cpp)

add_executable(run_tests test/test_main.cpp
                         test/test_cpu.cpp
                         test/test_instruction.cpp
                         test/test_timer.cpp
                         src/cpu.cpp
                         src/graphics.cpp
                         src/input.cpp
                         src/instruction.cpp
                         src/timer.cpp)

add_executable(fuzz src/fuzzing_main.cpp
                    src/cpu.cpp
                    src/graphics.cpp
                    src/input.cpp
                    src/instruction.cpp
                    src/timer.cpp)

# Multithreading support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(chip8_vm Threads::Threads)

# Enable libFuzzer and UBSAN
target_compile_options(fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
target_link_options(fuzz PRIVATE -fsanitize=fuzzer,address,undefined)

# LTO doesn't work with Clang 10 for whaterver reason when using libFuzzer
set_property(TARGET fuzz PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

# Header directories
include_directories(include)
target_include_directories(run_tests PRIVATE lib)
