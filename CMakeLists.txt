# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.25)

project (channel LANGUAGES CXX
                  VERSION 1.0.0)

include(cmake/CPM.cmake)
CPMAddPackage("gh:SGSSGene/cpmpack@1.0.2")
loadCPMPack("${CMAKE_CURRENT_SOURCE_DIR}/cpmpack.json")

#add_subdirectory(src/channel)
if (PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(src/demo_channel)
    add_subdirectory(src/test_channel)
endif()
