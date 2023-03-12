# * Copyright (C) 2023 Benjamin Thomas Schwertfeger
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see <https://www.gnu.org/licenses/>.
# ================================================================================

cmake_minimum_required(VERSION 3.10)

project(BiasAdjustCXX
    VERSION 1.8
    DESCRIPTION "Bias correction command-line tool for NetCDF-based climate data"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_CXX_COMPILER g++)

# add_compile_options(-Wall -v)
# find_package(netCDFCxx REQUIRED) # missing in cmake modules list (2022)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
include(FindNetCDFCxx)

if(netCDFCxx_FOUND)
    message (STATUS "NetCDFCxx found!")
    include_directories(${NetCDFCxx_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find NetCDFCxx!")
endif()


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

# set sources
set(SOURCES
    src/main.cxx
    src/CMethods.cxx
    src/Utils.cxx
    src/NcFileHandler.cxx
    src/MathUtils.cxx
    src/Manager.cxx
)

set(TEST_SOURCES
    tests/src/TestUtils.cxx
    tests/src/TestMathUtils.cxx
    tests/src/main.cxx
    src/CMethods.cxx
    src/Utils.cxx
    src/NcFileHandler.cxx
    src/MathUtils.cxx
    src/Manager.cxx
)

# add executable
add_executable( BiasAdjustCXX ${SOURCES})
add_executable( TestBiasAdjustCXX ${TEST_SOURCES})

# Include the netcdf headers
target_include_directories( BiasAdjustCXX
    PUBLIC ${netCDFCxx_INCLUDE_DIRS}
    PRIVATE include
)
target_include_directories( TestBiasAdjustCXX
    PUBLIC ${netCDFCxx_INCLUDE_DIRS}
    PRIVATE include
)

target_link_libraries( BiasAdjustCXX
    PUBLIC
        ${netCDFCxx_LIBRARIES}
)

target_link_libraries( TestBiasAdjustCXX
    GTest::gtest_main
    ${netCDFCxx_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(TestBiasAdjustCXX)
