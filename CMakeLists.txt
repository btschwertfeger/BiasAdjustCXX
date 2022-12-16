# * Copyright (C) 2022 Benjamin Thomas Schwertfeger
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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER g++)

# add_compile_options(-Wall -v)
# find_package(netCDFCxx REQUIRED) # missing in cmake modules list (2022)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
include(FindNetCDFCxx)

if(netCDFCxx_FOUND)
    message (STATUS "netCDFCxx found!")
    include_directories(${NetCDFCxx_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find netCDFCxx!")
endif()

# set sources
set(SOURCES
    src/main.cxx
    src/CMethods.cxx
    src/Utils.cxx
    src/NcFileHandler.cxx
    src/MathUtils.cxx
)

# add executable
add_executable( BiasAdjustCXX ${SOURCES})

# Include the netcdf headers
target_include_directories( BiasAdjustCXX
    PUBLIC ${netCDFCxx_INCLUDE_DIRS}
    PRIVATE include
)

target_link_libraries( BiasAdjustCXX
    PUBLIC
        ${netCDFCxx_LIBRARIES}
)
