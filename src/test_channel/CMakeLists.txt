# SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.25)

project(test_channel)

# fmindex_collection library
add_executable(${PROJECT_NAME}
    main.cpp
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    channel::channel
    Catch2::Catch2WithMain
)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
