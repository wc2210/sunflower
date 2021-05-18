########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.6)

project(sunflower CXX)
message(STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM} )
message(STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR} )
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "Build type not specified: defaulting to Debug.")
endif(NOT CMAKE_BUILD_TYPE)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
########################################################################
# Options
########################################################################

########################################################################
# Compiler specific setup
########################################################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-comment -Wno-reorder -Wno-unused-variable -Wtype-limits -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-comment -Wno-write-strings -Winline -Wno-unused-result -Wformat -Wmissing-field-initializers -Wtype-limits -std=c99 -D_GNU_SOURCE")
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")
else (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

message(STATUS "CMAKE_C_FLAGS is ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
########################################################################
# Add general global variable
########################################################################
#set(BASE "${PROJECT_SOURCE_DIR}/include")
########################################################################
# Add general includes and dependencies
########################################################################
include_directories(${PROJECT_SOURCE_DIR})
########################################################################
# Add the subdirectories
########################################################################
add_subdirectory(base)
add_subdirectory(example)


