// -*- lsst-c++ -*-

/**
 * @file ComputeIndicator.cxx
 * @brief Program to compute data sets containing statistical indicator results (like MBE, RMSE, ...)
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
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * ----- ----- ----- I N C L U D E ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */
#include <math.h>

#include <algorithm>
#include <vector>

#include "MathUtils.hxx"
#include "NcFileHandler.hxx"
#include "Utils.hxx"
#include "colors.h"

/*
 * ----- ----- ----- D E F I N I T I O N S ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

static std::vector<NcFileHandler*> v_NcFileHandlers;
static std::vector<std::string> v_input_filepaths;
static std::string
    method_name = "",
    variable_name = "",
    output_filepath = "";

static utils::Log Log = utils::Log();

/*
 * ----- ----- ----- P R O G R A M - M A N A G E M E N T ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

static bool isInStrV(std::vector<std::string> v, std::string string) {
    if (std::find(v.begin(), v.end(), string) != v.end())
        return true;
    else
        return false;
}

/** Prints usage hint for this program
 *
 * @param name name of this file
 */
static void show_usage(std::string name) {
    std::cout << BOLDBLUE << "Usage: " RESET << name
              << GREEN << " -i " << RESET << "inputfile1.nc"
              << GREEN << " -i" << RESET << " inputfile2.nc"
              << GREEN << " -o " << RESET << "outputfile.nc"
              << GREEN << " -v " << RESET << "temperature"
              << GREEN << " -m " << RESET << "rmse\n"
              << BOLDBLUE << "Parameters:\n"
              << RESET
              << "    required:\n"
              << GREEN << "\t-i, --input, \t\t" << RESET << "Inputfile / Filepath\n"
              << GREEN << "\t-o, --output\t\t" << RESET << "Outputfile / Filepath\n"
              << GREEN << "\t-v, --variable\t\t" << RESET << "Variablename (e.g.: tas, tsurf, pr) \n"
              << "    optional:\n"
              << RESET
              << "\tNone\n"
              << BOLDBLUE << "Requirements: \n"
              << RESET << "\t-> Data must 3-dimensional an dimensions in the following order: [time][lat][lon] and values type int or float\n"
              << "\t-> Latitudes and longitudes must be named 'lat' and 'lon', Time == 'time'\n"
              << BOLDBLUE << "Available methods: \n"
              << RESET;

    std::cout << YELLOW << "  ... for one inputfile:\n"
              << RESET;
    for (unsigned i = 0; i < MathUtils::requires_1_ds.size(); i++)
        std::cout << "\t-> " << MathUtils::requires_1_ds[i] << "\n";

    std::cout << YELLOW << "  ... for two inputfiles:\n"
              << RESET;
    for (unsigned i = 0; i < MathUtils::requires_2_ds.size(); i++)
        std::cout << "\t-> " << MathUtils::requires_2_ds[i] << "\n";
    std::cout << std::endl;
}

static void parse_args(int argc, char** argv) {
    if (argc == 1) {
        show_usage(argv[0]);
        exit(0);
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--input") {
            if (i + 1 < argc)
                v_input_filepaths.push_back(argv[++i]);
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-v" || arg == "--variable") {
            if (i + 1 < argc)
                variable_name = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-m" || arg == "--method") {
            if (i + 1 < argc)
                method_name = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc)
                output_filepath = argv[++i];
            else
                std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-h" || arg == "--help") {
            show_usage(argv[0]);
            exit(0);
        } else if (arg == "show") {
            if (i + 1 < argc) {
                if (std::string(argv[++i]).compare(std::string("-c")) == 0) {
                    utils::show_license();
                    exit(0);
                } else
                    throw std::runtime_error("Unknown flag " + std::string(argv[i]));
            } else
                throw std::runtime_error(arg + " requires one argument.");

        } else
            Log.warning("Unknown argument " + arg + "!");
    }

    if (variable_name.empty())
        throw std::runtime_error("No variable name defined!");
    else if (static_cast<int>(v_input_filepaths.size()) == 0)
        throw std::runtime_error("No inputfile(s) defined!");
    else if (output_filepath.empty())
        throw std::runtime_error("No outputfile defined!");
    else if (method_name.empty())
        throw std::runtime_error("No method specified!");
    else {
        // ? check if inputfiles match method requirements
        if (isInStrV(MathUtils::requires_1_ds, method_name) && v_input_filepaths.size() != 1)
            throw std::runtime_error("Method " + method_name + " requires 1 inputfile!");
        else if (isInStrV(MathUtils::requires_2_ds, method_name) && v_input_filepaths.size() != 2)
            throw std::runtime_error("Method " + method_name + " requires 2 inputfiles!");

        // ? initialize the data manager
        for (auto const& e : v_input_filepaths)
            v_NcFileHandlers.push_back(new NcFileHandler(e, variable_name, 3));
    }
}

template <typename T>
static void stdcout_runtime(T start_time) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end_time - start_time;
    std::cout << ms_double.count() << "ms\n";
}

/*
 * ----- ----- ----- C O M P U T A T I O N ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * ? Computes the results for a col; col = all latitudes for one longitude
 *
 * @param v_data_out reference to output data array
 * @param lon longitude to compute
 */
void compute_col_for_one_file(std::vector<std::vector<float>>& data_out, unsigned lon) {
    std::vector<std::vector<float>> dataset_lats(
        (int)v_NcFileHandlers[0]->n_lat,
        std::vector<float>((int)v_NcFileHandlers[0]->n_time)
    );

    v_NcFileHandlers[0]->get_lat_timeseries_for_lon(dataset_lats, lon);
    Func_one method = MathUtils::get_method_for_1_ds(method_name);

    for (unsigned lat = 0; lat < v_NcFileHandlers[0]->n_lat; lat++)
        data_out[lat][lon] = (float)method(dataset_lats[lat]);
}

/**
 *  ? Computes the results for a column; column = all latitudes for one longitudee
 *
 * @param v_data_out reference to output data vector
 * @param lon longitude to compute
 */
void compute_col_for_two_files(std::vector<std::vector<float>>& v_data_out, unsigned lon) {
    std::vector<std::vector<float>> dataset_1_lats(
        (int)v_NcFileHandlers[0]->n_lat,
        std::vector<float>((int)v_NcFileHandlers[0]->n_time)
    );
    std::vector<std::vector<float>> dataset_2_lats(
        (int)v_NcFileHandlers[1]->n_lat,
        std::vector<float>((int)v_NcFileHandlers[1]->n_time)
    );

    v_NcFileHandlers[0]->get_lat_timeseries_for_lon(dataset_1_lats, lon);
    v_NcFileHandlers[1]->get_lat_timeseries_for_lon(dataset_2_lats, lon);

    Func_two method = MathUtils::get_method_for_2_ds(method_name);
    for (unsigned lat = 0; lat < v_NcFileHandlers[0]->n_lat; lat++)
        v_data_out[lat][lon] = (float)method(dataset_1_lats[lat], dataset_2_lats[lat]);
}

/**
 * ? Computes the selected indicator for all latitudes / longitudes
 *
 * @param v_data_out output array
 */
void compute_indicator(std::vector<std::vector<float>>& v_data_out) {
    if (isInStrV(MathUtils::requires_1_ds, method_name)) {
        // * One dataset as input
        for (unsigned lon = 0; lon < v_NcFileHandlers[0]->n_lon; lon++) {
            compute_col_for_one_file(v_data_out, lon);
            utils::progress_bar((float)lon, (float)v_NcFileHandlers[0]->n_lon);
        }
    } else if (isInStrV(MathUtils::requires_2_ds, method_name)) {
        // * Two datasets as input
        for (unsigned lon = 0; lon < v_NcFileHandlers[0]->n_lon; lon++) {
            compute_col_for_two_files(v_data_out, lon);
            utils::progress_bar((float)lon, (float)v_NcFileHandlers[0]->n_lon);
        }
    }
    utils::progress_bar((float)v_NcFileHandlers[0]->n_lon, (float)v_NcFileHandlers[0]->n_lon);
    std::cout << std::endl;
}

/*
 * ----- ----- ----- M A I N  ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

int main(int argc, char** argv) {
    auto start_time = std::chrono::high_resolution_clock::now();
    utils::show_copyright_notice("ComputeIndicatorCXX");

    try {
        parse_args(argc, argv);
        Log.info("Method: " + method_name);
        std::vector<std::vector<float>> v_data_out(
            (int)v_NcFileHandlers[0]->n_lat,
            std::vector<float>((int)v_NcFileHandlers[0]->n_lon)
        );

        Log.info("Starting computation!");
        compute_indicator(v_data_out);

        Log.info("Saving: " + output_filepath);
        v_NcFileHandlers[0]->to_netcdf(output_filepath, method_name, v_data_out);

        Log.info("SUCCESS!");
    } catch (const std::runtime_error& error) {
        Log.error(error.what());
        stdcout_runtime(start_time);
        exit(1);
    }
    return 0;
}

/**
 * * ----- ----- E O F ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ |
 */