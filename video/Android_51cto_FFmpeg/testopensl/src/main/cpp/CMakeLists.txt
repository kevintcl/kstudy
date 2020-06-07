
cmake_minimum_required(VERSION 3.4.1)

add_library(
        audio-lib
        SHARED
        audio.cpp
)


find_library(
        log-lib

        log)
target_link_libraries(
        audio-lib
        OpenSLES

        ${log-lib})