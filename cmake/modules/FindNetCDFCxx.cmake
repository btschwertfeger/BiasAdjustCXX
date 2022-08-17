# 
# Find the netCDF C++ API
# 
# ----- R E F E R E N C E S -----
# Inspired by:
#   Peter Hill (2021) 
#   @Source: https://gitlab.mpcdf.mpg.de/dave/BOUT-dev/-/blob/manylinux/cmake/FindnetCDFCxx.cmake
#
# This module uses the ``ncxx4-config`` helper script as a hint for
# the location of the NetCDF C++ library. It should be in your PATH.
#
# Exports:
#   netCDFCxx_FOUND - true || false if netCDFCxx was found
#   netCDFCxx_INCLUDE_DIRS - path of netCDFCxx includes
#   netCDFCxx_LIBRARIES - netCDFCxx libraries

find_package(netCDF REQUIRED)
find_program(NCXX4_CONFIG "ncxx4-config")

get_filename_component(NCXX4_CONFIG_TMP "${NCXX4_CONFIG}" DIRECTORY)
get_filename_component(NCXX4_CONFIG_LOCATION "${NCXX4_CONFIG_TMP}" DIRECTORY)

find_path(netCDF_CXX_INCLUDE_DIR NAMES netcdf)
find_library(netCDF_CXX_LIBRARY NAMES netcdf_c++4 netcdf-cxx4)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(netCDFCxx
    REQUIRED_VARS netCDF_CXX_LIBRARY netCDF_CXX_INCLUDE_DIR
    VERSION_VAR netCDFCxx_VERSION
)

if (netCDFCxx_FOUND)
    set(netCDFCxx_INCLUDE_DIRS "${netCDF_CXX_INCLUDE_DIR}")
    set(netCDFCxx_LIBRARIES "${netCDF_CXX_LIBRARY}")
endif ()