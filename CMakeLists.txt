cmake_minimum_required(VERSION 3.15)

project(kinoubi LANGUAGES C)

if(NOT DEFINED ENV{GBDK_HOME})
    message(FATAL_ERROR "GBDK_HOME not set.")
endif()
set(GBDK_HOME $ENV{GBDK_HOME})

if(DEFINED ENV{CLION_IDE})
    message(STATUS "Detected CLion environment. Using GCC/Clang for code insight.")
    add_definitions(-D__CLION_IDE__)
    set(CMAKE_C_COMPILER gcc)
else()
    message(STATUS "Building with GBDK's lcc compiler.")
    set(CMAKE_C_COMPILER ${GBDK_HOME}/bin/lcc)
    set(CMAKE_C_FLAGS "")
    set(CMAKE_C_FLAGS_DEBUG "")
    set(CMAKE_C_FLAGS_RELEASE "")
    set(CMAKE_DEPFILE_FLAGS_C "")
endif()

file(GLOB_RECURSE SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.c"
        "${PROJECT_SOURCE_DIR}/res/*.c"
)

include_directories(
        ${GBDK_HOME}/include
        "include"
        "."
        "res"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wm-yC -Wf-MMD -Wf-Wp-MP -Wl-yt0x19 -autobank -Wb-v -Wb-ext=.rel")

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} SUFFIX ".gbc")