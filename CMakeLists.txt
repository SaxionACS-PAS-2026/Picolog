cmake_minimum_required(VERSION 3.20...3.27)

project(lib_picolog)

add_library(lib_picolog INTERFACE)

target_include_directories(lib_picolog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(lib_picolog INTERFACE cxx_std_20)

target_compile_definitions(lib_picolog INTERFACE PICOLOG_ROOT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_library(picolog::picolog ALIAS lib_picolog)

add_subdirectory(demo)