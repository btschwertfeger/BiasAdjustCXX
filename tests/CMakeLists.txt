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

set(BINARY Test${CMAKE_PROJECT_NAME})

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
