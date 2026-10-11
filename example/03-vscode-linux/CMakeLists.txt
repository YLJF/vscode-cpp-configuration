cmake_minimum_required(VERSION 3.16)
project(AcmeCalc VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ACME_BUILD_TOOLS "Build the information tool" OFF)
set(ACME_APP_LABEL "AcmeCalc" CACHE STRING "Application label")

add_subdirectory(lib)

add_executable(acme_calc app/calc.cpp)
target_link_libraries(acme_calc PRIVATE acme_math)

if(ACME_BUILD_TOOLS)
    add_executable(acme_info app/info.cpp)
    target_link_libraries(acme_info PRIVATE acme_math)
endif()
