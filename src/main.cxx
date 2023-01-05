// -*- lsst-c++ -*-

/**
 * @file main.cxx
 * @brief Main program to bias adjust NetCDF-based climate data using the `BiasAdjustCXX` command-line tool
 * @author Benjamin Thomas Schwertfeger
 * @email: development@b-schwertfeger.de
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/BiasAdjustCXX
 *
 * * Copyright (C) 2023 Benjamin Thomas Schwertfeger
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * * Description
 *      Main program to bias adjust climate data;
 *      - loads data sets
 *      - iteration over all grid cells (if inputs are 3-dimensional data sets)
 *      - application/execution of the selected adjustment
 *      - save results to .nc file
 *
 * * Compilation:
 *    use the CMakeLists.txt for os-independand compilation.
 */

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Includes
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include <math.h>

#include <chrono>

#include "CMethods.hxx"
#include "Manager.hxx"
#include "Utils.hxx"

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Helper functions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * Command-line output of the duraton between a start time
 *
 * @param start_time start time to calculate the duration
 */
template <typename T>
static void stdcout_runtime(T start_time) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end_time - start_time;
    std::cout << "Runtime: " << ms_double.count() << "ms\n";
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Main
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

int main(int argc, char** argv) {
    auto start_time = std::chrono::high_resolution_clock::now();
    utils::show_copyright_notice("BiasAdjustCXX");
    utils::Log log = utils::Log();

    try {
        Manager manager = Manager(argc, argv);
        manager.run_adjustment();

        stdcout_runtime(start_time);
        return 0;
        // } catch (const netCDF::exceptions::NcInvalidCoords& error) {
        // } catch (const netCDF::exceptions::NcBadId& error) {
        // } catch (const netCDF::exceptions::NcBadDim& error) {
    } catch (const netCDF::exceptions::NcException& error) {
        log.error(error.what());
        log.info(
            "Please check:\n"
            "    - the dimensions of the input files match:\n"
            "       - 3-dimensional: time, lat, lon | without '--1dim' flag\n"
            "       - 1-dimensional: time           | '--1dim' flag is required\n"
            "    - the resolutions of the input files must be the same."
            "    - the variable has the same name in all input files.");
        stdcout_runtime(start_time);
        return 1;
    } catch (const std::runtime_error& error) {
        log.error(error.what());
        stdcout_runtime(start_time);
        return 1;
    } catch (const std::exception& error) {
        log.error(error.what());
        return 1;
    }
}
