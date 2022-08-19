cmake_minimum_required(VERSION 3.10)

project(BiasAdjustmentCpp
    VERSION 1.0
    DESCRIPTION "Bias Adjustment"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER g++)

# add_compile_options(-Wall -v)

# find_package(netCDFCxx REQUIRED) # missing in cmake modules list
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
include(FindnetCDFCxx)

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
    src/MyMath.cxx
)

# add executable
add_executable( Main ${SOURCES})

# Include the netcdf headers
target_include_directories( Main
    PUBLIC ${netCDFCxx_INCLUDE_DIRS}
    PRIVATE include
)

target_link_libraries( Main
    PUBLIC
        ${netCDFCxx_LIBRARIES} 
)