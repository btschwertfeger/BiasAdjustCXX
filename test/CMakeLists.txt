# * Copyright (C) 2022  Benjamin Thomas Schwertfeger
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
# ================================================================================

cmake_minimum_required(VERSION 3.10)

project(ComputeIndicatorCXX
    VERSION 1.0
    DESCRIPTION "Command line tool to compute a 2-dimensional matrice based on inputfile(s)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER g++)

# add_compile_options(-Wall -v)
# find_package(netCDFCxx REQUIRED) # missing in cmake modules list (2022)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/modules/")
include(FindNetCDFCxx)

if(netCDFCxx_FOUND)
    message (STATUS "netCDFCxx found!")
    include_directories(${NetCDFCxx_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find netCDFCxx!")
endif()

# set sources
set(SOURCES
    src/ComputeIndicator.cxx
    ../src/Utils.cxx
    ../src/NcFileHandler.cxx
    ../src/MathUtils.cxx
)

# add executable
add_executable( ComputeIndicatorCXX ${SOURCES})

# Include the netcdf headers
target_include_directories( ComputeIndicatorCXX
    PUBLIC ${netCDFCxx_INCLUDE_DIRS}
    PRIVATE ../include
)

target_link_libraries( ComputeIndicatorCXX
    PUBLIC
        ${netCDFCxx_LIBRARIES}
)
