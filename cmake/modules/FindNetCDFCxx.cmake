# 
# Find the netCDF C++ API cmake files
# @github https://github.com/btschwertfeger/BiasAdjustCXX
#
# * Copyright (C) 2023 Benjamin Thomas Schwertfeger
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
#
# ----- R E F E R E N C E S -----
# Inspired by:
#   Peter Hill (2021) 
#   @source: https://gitlab.mpcdf.mpg.de/dave/BOUT-dev/-/blob/manylinux/cmake/FindnetCDFCxx.cmake
#
# Exports the following variables:
#   netCDFCxx_FOUND - true || false if netCDFCxx was found
#   netCDFCxx_INCLUDE_DIRS - path of netCDFCxx includes
#   netCDFCxx_LIBRARIES - netCDFCxx libraries

message (STATUS "searching netCDFCxx...")
# find_package(netCDF REQUIRED) # this causes errors on CXX compiler with identification GNU 4.8.5
find_program(NCXX4_CONFIG "ncxx4-config")

get_filename_component(NCXX4_CONFIG_TMP "${NCXX4_CONFIG}" DIRECTORY)
get_filename_component(NCXX4_CONFIG_LOCATION "${NCXX4_CONFIG_TMP}" DIRECTORY)

find_path(netCDF_CXX_INCLUDE_DIR NAMES netcdf
    HINTS
     "${NCXX4_CONFIG_LOCATION}"
    PATH_SUFFIXES
     "include"
)
find_library(netCDF_CXX_LIBRARY NAMES netcdf_c++4 netcdf-cxx4
    HINTS 
     "${NCXX4_CONFIG_LOCATION}"
    PATH_SUFFIXES
     "lib" "lib64"
)

include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(netCDFCxx
    REQUIRED_VARS netCDF_CXX_LIBRARY netCDF_CXX_INCLUDE_DIR
    VERSION_VAR netCDFCxx_VERSION
)

if (netCDFCxx_FOUND)
    set(netCDFCxx_INCLUDE_DIRS "${netCDF_CXX_INCLUDE_DIR}")
    set(netCDFCxx_LIBRARIES "${netCDF_CXX_LIBRARY}")
endif ()
