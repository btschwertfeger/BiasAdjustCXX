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

project(TestBiasAdjustCXX
    VERSION 1.8.2
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# find_package(netCDFCxx REQUIRED) # missing in cmake modules list (2022)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/modules/")
include(FindNetCDFCxx)

if(netCDFCxx_FOUND)
    message (STATUS "NetCDFCxx found!")
    include_directories(${NetCDFCxx_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find NetCDFCxx!")
endif()

set(BINARY ${CMAKE_PROJECT_NAME})

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
    src/TestUtils.cxx
    src/TestMathUtils.cxx
    src/TestCMethods.cxx
    src/TestManager.cxx
    src/TestNcFileHandler.cxx
    src/main.cxx
    ../src/CMethods.cxx
    ../src/Utils.cxx
    ../src/NcFileHandler.cxx
    ../src/MathUtils.cxx
    ../src/Manager.cxx
)

# add executable
add_executable(${BINARY} ${TEST_SOURCES})

target_include_directories(${BINARY}
    PUBLIC ${netCDFCxx_INCLUDE_DIRS}
    PRIVATE ../include
)

target_link_libraries( ${BINARY}
    GTest::gtest_main
    ${netCDFCxx_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(${BINARY})

# Uninstall target
add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_MODULE_PATH}/uninstall.cmake"
)
