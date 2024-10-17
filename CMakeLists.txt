cmake_minimum_required(VERSION 3.10)

project(kinoubi LANGUAGES C)

if(NOT DEFINED ENV{GBDK_HOME})
    message(FATAL_ERROR "Die Umgebungsvariable GBDK_HOME ist nicht gesetzt.")
endif()
set(GBDK_HOME $ENV{GBDK_HOME})

if(DEFINED ENV{CLION_IDE})
    message(STATUS "Detected CLion environment. Using GCC/Clang for code insight.")

    set(CMAKE_C_COMPILER gcc)
else()
    message(STATUS "Building with GBDK's lcc compiler.")

    set(CMAKE_C_COMPILER ${GBDK_HOME}/bin/lcc)

    set(CMAKE_C_FLAGS "")
    set(CMAKE_C_FLAGS_DEBUG "")
    set(CMAKE_C_FLAGS_RELEASE "")
    set(CMAKE_DEPFILE_FLAGS_C "")
endif()

include_directories(${GBDK_HOME}/include)

file(GLOB SOURCES
        src/*.c
)
add_executable(kinoubi ${SOURCES})

add_custom_command(TARGET kinoubi
        POST_BUILD
        COMMAND ${CMAKE_C_COMPILER} -Wa-l -Wl-m -o ${CMAKE_BINARY_DIR}/kinoubi.gb ${SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building GB ROM"
)

add_custom_command(TARGET kinoubi
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/kinoubi.gb ${BOTTLE_C}
)