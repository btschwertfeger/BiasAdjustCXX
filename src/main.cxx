// -*- lsst-c++ -*-

/**
 * @file main.cxx
 * @brief Main program to bias adjust NetCDF-based climate data
 * @author Benjamin Thomas Schwertfeger
 * @email: development@b-schwertfeger.de
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 *
 * * Copyright (C) 2022  Benjamin Thomas Schwertfeger
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * * Description
 *      Main program to bias adjust climate data;
 *      - loads data sets
 *      - iteration over all grid cells (if 3-dimensional data set)
 *      - application/execution of the selected adjustment
 *      - save results to .nc file
 *
 * * Compilaition on macOS 12.4:
 *      g++ -std=c++11 -Wall -v     \
 *          src/main.cxx            \
 *          -o Main.app             \
 *          src/Utils.cxx           \
 *          src/MyMath.cxx          \
 *          src/CMethods.cxx        \
 *          src/NcFileHandler.cxx   \
 *          -I include              \
 *          -lnetcdf-cxx4
 * * ... or use the CMakeLists.txt file.
 */

/*
 * ----- ----- ----- I N C L U D E ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include <math.h>

#include <chrono>
#include <vector>

#include "CMethods.hxx"
#include "NcFileHandler.hxx"
#include "Utils.hxx"
#include "colors.h"
/*
 * ----- ----- ----- D E F I N I T I O N S ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

static NcFileHandler
    ds_reference,
    ds_control,
    ds_scenario;

static std::string
    variable_name = "",
    output_filepath = "",
    adjustment_method_name = "",
    adjustment_kind = "add";

static CM_Func_ptr_scaling scaling_func = NULL;
static CM_Func_ptr_distribution distribution_func = NULL;

static unsigned n_quantiles = 100;
static bool one_dim = false;
static utils::Log Log = utils::Log();

/*
 * ----- ----- ----- P R O G R A M - M A N A G E M E N T ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

static void parse_args(int argc, char** argv) {
    if (argc == 1) {
        utils::show_usage(argv[0]);
        exit(0);
    }
    std::string
        reference_fpath = "",
        control_fpath = "",
        scenario_fpath = "";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--ref" || arg == "--reference") {
            if (i + 1 < argc)
                reference_fpath = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");

        } else if (arg == "--contr" || arg == "--control") {
            if (i + 1 < argc)
                control_fpath = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");

        } else if (arg == "--scen" || arg == "--scenario") {
            if (i + 1 < argc)
                scenario_fpath = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-v" || arg == "--variable") {
            if (i + 1 < argc)
                variable_name = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-q" || arg == "--quantiles") {
            if (i + 1 < argc)
                n_quantiles = (unsigned)std::stoi(argv[++i]);
        } else if (arg == "-m" || arg == "--method") {
            if (i + 1 < argc) {
                adjustment_method_name = argv[++i];
                scaling_func = CMethods::get_cmethod_scaling(adjustment_method_name);
                distribution_func = CMethods::get_cmethod_distribution(adjustment_method_name);
            } else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-k" || arg == "--kind") {
            if (i + 1 < argc)
                adjustment_kind = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc)
                output_filepath = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--1dim")
            one_dim = true;
        else if (arg == "-h" || arg == "--help") {
            utils::show_usage(argv[0]);
            exit(0);
        } else if (arg == "show") {
            if (i + 1 < argc) {
                if (std::string(argv[++i]).compare(std::string("-c")) == 0) {
                    utils::show_license();
                    exit(1);
                } else
                    throw std::runtime_error("Unknown flag " + std::string(argv[i]));
            } else
                throw std::runtime_error(arg + " requires one argument.");
        } else
            Log.warning("Unknown argument: " + arg + "!");
    }
    if (variable_name.empty())
        throw std::runtime_error("No variable name defined!");
    else if (reference_fpath.empty())
        throw std::runtime_error("No reference file defined!");
    else if (control_fpath.empty())
        throw std::runtime_error("No control file defined!");
    else if (scenario_fpath.empty())
        throw std::runtime_error("No scenario file defined!");
    else if (output_filepath.empty())
        throw std::runtime_error("No outputfile defined!");
    else if (adjustment_method_name.empty())
        throw std::runtime_error("No method specified!");
    else if (adjustment_kind.empty())
        throw std::runtime_error("Adjustmend kind is empty!");
    else {
        if (one_dim) {
            ds_reference = NcFileHandler(reference_fpath, variable_name, 1),
            ds_control = NcFileHandler(control_fpath, variable_name, 1),
            ds_scenario = NcFileHandler(scenario_fpath, variable_name, 1);
        } else {
            ds_reference = NcFileHandler(reference_fpath, variable_name, 3),
            ds_control = NcFileHandler(control_fpath, variable_name, 3),
            ds_scenario = NcFileHandler(scenario_fpath, variable_name, 3);

            if (ds_reference.n_lat != ds_control.n_lat || ds_reference.n_lat != ds_scenario.n_lat)
                throw std::runtime_error("Latitude dimension of input files does not have the same length!");
            else if (ds_reference.n_lon != ds_control.n_lon || ds_reference.n_lon != ds_scenario.n_lon)
                throw std::runtime_error("Longitude dimension of input files does not have the same length!");
        }
    }

    if (ds_reference.n_time != ds_control.n_time || ds_reference.n_time != ds_scenario.n_time) {
        if (adjustment_method_name == std::string("delta_method"))
            throw std::runtime_error("Time dimension input files does not have the same length! This is required for the delta method.");
    }
}

template <typename T>
static void stdcout_runtime(T start_time) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end_time - start_time;
    std::cout << ms_double.count() << "ms\n";
}

/*
 * ----- ----- ----- C O M P U T A T I O N ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

static void adjust_1d(
    std::vector<float>& v_data_out,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario) {
    if (scaling_func != NULL) {
        scaling_func(
            v_data_out,
            v_reference,
            v_control,
            v_scenario,
            adjustment_kind);
    } else if (distribution_func != NULL) {
        distribution_func(
            v_data_out,
            v_reference,
            v_control,
            v_scenario,
            adjustment_kind,
            n_quantiles);
    } else
        throw std::runtime_error("Unknown adjustment method " + adjustment_method_name + "!");
}

static void adjust_3d(std::vector<std::vector<std::vector<float>>>& v_data_out) {
    for (unsigned lon = 0; lon < ds_scenario.n_lon; lon++) {
        utils::progress_bar((float)lon, (float)(v_data_out[0].size()));

        std::vector<std::vector<float>> v_reference_lat_data(
            (int)ds_reference.n_lat,
            std::vector<float>((int)ds_reference.n_time));

        std::vector<std::vector<float>> v_control_lat_data(
            (int)ds_control.n_lat,
            std::vector<float>((int)ds_control.n_time));

        std::vector<std::vector<float>> v_scenario_lat_data(
            (int)ds_scenario.n_lat,
            std::vector<float>((int)ds_scenario.n_time));

        ds_reference.get_lat_timeseries_for_lon(v_reference_lat_data, lon);
        ds_control.get_lat_timeseries_for_lon(v_control_lat_data, lon);
        ds_scenario.get_lat_timeseries_for_lon(v_scenario_lat_data, lon);

        for (unsigned lat = 0; lat < ds_scenario.n_lat; lat++)
            adjust_1d(
                v_data_out[lat][lon],
                v_reference_lat_data[lat],
                v_control_lat_data[lat],
                v_scenario_lat_data[lat]);
    }
    utils::progress_bar((float)(v_data_out[0].size()), (float)(v_data_out[0].size()));
    std::cout << std::endl;
}

/*
 * ----- ----- ----- M A I N ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

int main(int argc, char** argv) {
    auto start_time = std::chrono::high_resolution_clock::now();
    std::cerr << "BiasAdjustmentCpp  Copyright (C) 2022 Benjamin Thomas Schwertfeger"
              << "\nThis program comes with ABSOLUTELY NO WARRANTY."
              << "\nThis is free software, and you are welcome to redistribute it"
              << "\nunder certain conditions; type `show -c' for details."
              << "\n"
              << std::endl;
    try {
        parse_args(argc, argv);

        if (one_dim) {  // adjustment of data set containing only one grid cell
            std::vector<float>
                v_data_out((int)ds_scenario.n_time),
                v_reference((int)ds_reference.n_time),
                v_control((int)ds_control.n_time),
                v_scenario((int)ds_scenario.n_time);

            ds_reference.get_timeseries(v_reference);
            ds_control.get_timeseries(v_control);
            ds_scenario.get_timeseries(v_scenario);

            adjust_1d(v_data_out, v_reference, v_control, v_scenario);
            std::cout << std::endl;
            Log.info("Saving " + output_filepath);
            ds_scenario.to_netcdf(output_filepath, variable_name, v_data_out);

        } else {  // adjustment of 3-dimensional data set
            // ? prepare lat x lon x time
            std::vector<std::vector<std::vector<float>>> v_data_out(
                (int)ds_scenario.n_lat,
                std::vector<std::vector<float>>(
                    (int)ds_scenario.n_lon,
                    std::vector<float>(
                        (int)ds_scenario.n_time)));

            adjust_3d(v_data_out);

            std::vector<std::vector<std::vector<float>>> v_data_to_save(
                (int)ds_scenario.n_time,
                std::vector<std::vector<float>>(
                    (int)ds_scenario.n_lat,
                    std::vector<float>(
                        (int)ds_scenario.n_lon)));

            // ? reshape to lat x lon x time
            for (unsigned lat = 0; lat < v_data_out.size(); lat++)
                for (unsigned lon = 0; lon < v_data_out[lat].size(); lon++)
                    for (unsigned time = 0; time < v_data_out[lat][lon].size(); time++)
                        v_data_to_save[time][lat][lon] = v_data_out[lat][lon][time];

            Log.info("Saving " + output_filepath);
            ds_scenario.to_netcdf(output_filepath, variable_name, v_data_to_save);
        }
        Log.info("SUCCESS!");
        stdcout_runtime(start_time);

    } catch (const std::runtime_error& error) {
        Log.error(error.what());
        stdcout_runtime(start_time);
        exit(1);
    }
    return 0;
}

/*
 * ----- ----- ----- E O F ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */
