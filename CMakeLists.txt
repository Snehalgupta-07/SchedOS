cmake_minimum_required(VERSION 3.14)
project(SchedOS VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Core Library ---
add_library(schedos_core
    src/Process.cpp
    src/ProcessManager.cpp
    src/Scheduler.cpp
    src/FCFS.cpp
    src/SJF.cpp
    src/SRTF.cpp
    src/RoundRobin.cpp
    src/PriorityScheduling.cpp
    src/Metrics.cpp
    src/CLI.cpp
    src/GanttChart.cpp
)
target_include_directories(schedos_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# --- Main Executable ---
add_executable(schedos src/main.cpp)
target_link_libraries(schedos PRIVATE schedos_core)

# --- Test Executable ---
add_executable(schedos_tests tests/test_main.cpp)
target_link_libraries(schedos_tests PRIVATE schedos_core)

if(MINGW)
    target_link_options(schedos PRIVATE -static -static-libgcc -static-libstdc++)
    target_link_options(schedos_tests PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Enable CTest
enable_testing()
add_test(NAME SchedulingTests COMMAND schedos_tests)
