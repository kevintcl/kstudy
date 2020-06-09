# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

include_directories(include)

set(FF /Users/tangchunlin/study/kstudy/video/Android_51cto_FFmpeg/xplay/libs/${ANDROID_ABI})

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FF}/libavcodec.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FF}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FF}/libavutil.so)


add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FF}/libswscale.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FF}/libswresample.so)

add_library(
        native-lib

        SHARED

        native-lib.cpp
        IDemux.cpp
        FFDemux.cpp
        XData.cpp
        XLog.cpp
)


find_library(
        log-lib

        log)


target_link_libraries(
        native-lib
        android
        avcodec
        avformat
        avutil
        swscale
        swresample
        ${log-lib})