cmake_minimum_required(VERSION 3.16)
project(debug_lab CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -g -O0)

add_executable(crash_demo src/crash_demo.cpp)
add_executable(race_demo src/race_demo.cpp)
add_executable(deadlock_demo src/deadlock_demo.cpp)
add_executable(perf_demo src/perf_demo.cpp)

find_package(Threads REQUIRED)
target_link_libraries(race_demo PRIVATE Threads::Threads)
target_link_libraries(deadlock_demo PRIVATE Threads::Threads)

add_executable(echo_server src/echo_server.cpp)

if(UNIX)
  target_link_libraries(echo_server PRIVATE Threads::Threads)
endif()
