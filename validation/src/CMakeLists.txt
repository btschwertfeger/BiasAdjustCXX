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

set(BINARY ${CMAKE_PROJECT_NAME})

# set sources
set(SOURCES
    ComputeIndicator.cxx
    ../../src/Utils.cxx
    ../../src/NcFileHandler.cxx
    ../../src/MathUtils.cxx
)

# add executable
add_executable(${BINARY} ${SOURCES})

# Include the netcdf headers
target_include_directories(${BINARY}
    PUBLIC ${netCDFCxx_INCLUDE_DIRS}
    PRIVATE ../../include
)

target_link_libraries(${BINARY}
    PUBLIC
        ${netCDFCxx_LIBRARIES}
)

install(TARGETS ${BINARY})

# Uninstall target
add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_MODULE_PATH}/uninstall.cmake"
)
