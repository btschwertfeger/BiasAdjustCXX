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

#include <math.h>

#include <chrono>
#include <vector>

#include "CMethods.hxx"
#include "NcFileHandler.hxx"
#include "Utils.hxx"
#include "colors.h"

/**
 * ------------------------------------------------------------
 *  ____        __ _       _ _   _
 * |  _ \  ___ / _(_)_ __ (_) |_(_) ___  _ __  ___
 * | | | |/ _ \ |_| | '_ \| | __| |/ _ \| '_ \/ __|
 * | |_| |  __/  _| | | | | | |_| | (_) | | | \__ \
 * |____/ \___|_| |_|_| |_|_|\__|_|\___/|_| |_|___/
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

static unsigned n_quantiles = 250;
static double max_scaling_factor = 10.0;
static bool
    one_dim = false,
    interval31_scaling = true;
static utils::Log Log = utils::Log();

typedef void (*CM_Func_ptr_scaling_A)(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    bool interval31_scaling);

typedef void (*CM_Func_ptr_scaling_B)(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    double max_scaling_factor,
    bool interval31_scaling);

typedef void (*CM_Func_ptr_distribution)(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    std::string kind,
    unsigned n_quantiles);

static CM_Func_ptr_scaling_A scaling_func_A = NULL;
static CM_Func_ptr_scaling_B scaling_func_B = NULL;
static CM_Func_ptr_distribution distribution_func = NULL;

/**
 * ------------------------------------------------------------------
 * ==== Program Management ======
 */
void show_usage(std::string name) {
    std::cerr << BOLDBLUE << "Usage: " RESET << name << "\t\t\t\\\n"
              << GREEN << "\t --ref " << RESET << "observation_data.nc\t\\\n"
              << GREEN << "\t --contr " << RESET << "control_data.nc\t\\\n"
              << GREEN << "\t --scen " << RESET << "data_to_adjust.nc\t\\\n"
              << GREEN << "\t -v " << RESET << "tas\t\t\t\t\\\n"
              << GREEN << "\t -m " << RESET << "linear_scaling\t\t\\\n"
              << GREEN << "\t -o " << RESET << "result_linear_scaling.nc\n\n"
              << BOLDBLUE << "Parameters:\n"
              << RESET
              << "    required:\n"
              << GREEN << "\t--ref, --reference\t\t" << RESET << "observation/reanalysis data => input file/file path\n"
              << GREEN << "\t--contr, --control\t\t" << RESET << "modeled control period data => input file/file path\n"
              << GREEN << "\t--scen, --scenario\t\t" << RESET << "modeled scenario period data to adjust => input file/file path\n"
              << GREEN << "\t-o, --output\t\t\t" << RESET << "output file/file path\n"
              << GREEN << "\t-v, --variable\t\t\t" << RESET << "variable name (e.g.: tas, tsurf, pr) \n"
              << "    optional:\n"
              << GREEN << "\t-h, --help\t\t\t" << RESET << "show this help message\n"
              << GREEN << "\t-q, --quantiles\t\t\t" << RESET << "number of quantiles to use when using a quantile adjustment method\n"
              << GREEN << "\t-k, --kind\t\t\t" << RESET << "kind of adjustment e.g.: '+' or '*' for additive or multiplicative method (default: '+')\n"
              << GREEN << "\t    --1dim\t\t\t" << RESET << "select this, when all input data sets only contain the time dimension (i.e. no spatial dimensions)\n"
              << GREEN << "\t    --monthly\t\t\t" << RESET << "disables the adjustment based on long-term 31-day intervals for the sclaing-based methods; "
                                                              "mean calculation will be performed on the whole data set\n"
              << GREEN << "\t    --max-scaling-factor\t" << RESET << "define the maximum scaling factor to avoid unrealistic results when adjusting ratio based variables "
                                                                     "(only for scaling methods; default: 10)"
              << "\n\n"
              << BOLDBLUE << "Requirements: \n"
              << RESET
              << "-> data sets must habe the file type NetCDF\n"
              << "-> for scaling-based adjustments: all input files must have 365 days per year (no February 29th.) otherwise the " << GREEN << "--monthly" << RESET << " flag is needed (see notes section below)\n"
              << "-> all data must be in format: [time][lat][lon] (if " << GREEN << "--1dim" << RESET << " is not slected) and values of type float\n"
              << "-> latitudes, longitudes and times must be named 'lat', 'lon' and 'time'\n"
              << RESET << std::endl;

    std::cerr << BOLDBLUE << "Available methods: " << RESET << "\n-> ";
    std::vector<std::string> all_methods;
    all_methods.reserve(CMethods::scaling_method_names.size() + CMethods::distribution_method_names.size());  // preallocate memory
    all_methods.insert(all_methods.end(), CMethods::scaling_method_names.begin(), CMethods::scaling_method_names.end());
    all_methods.insert(all_methods.end(), CMethods::distribution_method_names.begin(), CMethods::distribution_method_names.end());

    for (size_t i = 0; i < all_methods.size(); i++) std::cerr << all_methods[i] << " ";
    std::cout << std::endl;
    std::cerr << YELLOW << "\nNotes: " << RESET
              << "\n- When not using the " << GREEN << "--monthly" << RESET << " flag it is required that all input files must have 365 days per year (no February 29th.) "
              << "The Linear Scaling, Variance Scaling and Delta Method need a wrapper script when the " << GREEN << "--monthly" << RESET << " flag is used to apply this program on monthly separated files i.e. "
              << "to adjust 30 years of data, all input files need to be separated into 12 groups, one group for each month, than this program can be applied to every long-term month."
              << "\n\n- The Delta Method requires that the time series of the control period have the same length as the time series to be adjusted.";

    std::cerr << YELLOW << "\n\n====== References ======" << RESET
              << "\n- Copyright (C) Benjamin Thomas Schwertfeger (2022) development@b-schwertfeger.de"
              << "\n- Unidata's NetCDF Programming Interface NetCDFCxx Data structures: http://doi.org/10.5065/D6H70CW6"
              << "\n- Mathematical foundations:"
              << "\n(1) Beyer, R., Krapp, M., and Manica, A.: An empirical evaluation of bias correction methods for palaeoclimate simulations, Climate of the Past, 16, 1493–1508, https://doi.org/10.5194/cp-16-1493-2020, 2020"
              << "\n\n(2) Cannon, A. J., Sobie, S. R., and Murdock, T. Q.: Bias Correction of GCM Precipitation by Quantile Mapping: How Well Do Methods Preserve Changes in Quantiles and Extremes?, Journal of Climate, 28, 6938 – 6959, https://doi.org/10.1175/JCLI-D-14-00754.1, 2015."
              << "\n\n(3) Maraun, D.: Nonstationarities of Regional Climate Model Biases in European Seasonal Mean Temperature and Precipitation Sums, Geophysical Research Letters, 39, 6706–, https://doi.org/10.1029/2012GL051210, 2012."
              << "\n\n(4) Teutschbein, C. and Seibert, J.: Bias correction of regional climate model simulations for hydrological climate-change impact studies: Review and evaluation of different methods, Journal of Hydrology, s 456–457, 12–29, https://doi.org/10.1016/j.jhydrol.2012.05.052, 2012."
              << "\n\n(5) Tong, Y., Gao, X., Han, Z., Xu, Y., Xu, Y., and Giorgi, F.: Bias correction of temperature and precipitation over China for RCM simulations using the QM and QDM methods, Climate Dynamics, 57, https://doi.org/10.1007/s00382-020-05447-4, 2021.";
    std::cout.flush();
}

static std::string get_adjustment_kind() {
    return (adjustment_kind == "add" || adjustment_kind == "+")
               ? "add"
           : (adjustment_kind == "mult" || adjustment_kind == "*")
               ? "mult"
               : "";
}

static void parse_args(int argc, char** argv) {
    if (argc == 1) {
        show_usage(argv[0]);
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
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--contr" || arg == "--control") {
            if (i + 1 < argc)
                control_fpath = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--scen" || arg == "--scenario") {
            if (i + 1 < argc)
                scenario_fpath = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-v" || arg == "--variable") {
            if (i + 1 < argc)
                variable_name = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-q" || arg == "--quantiles") {
            if (i + 1 < argc)
                n_quantiles = (unsigned)std::stoi(argv[++i]);
        } else if (arg == "-m" || arg == "--method") {
            if (i + 1 < argc)
                adjustment_method_name = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-k" || arg == "--kind") {
            if (i + 1 < argc)
                adjustment_kind = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--max-scaling-factor") {
            if (i + 1 < argc)
                max_scaling_factor = std::stoi(argv[++i]);
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--monthly")
            interval31_scaling = false;
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc)
                output_filepath = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--1dim")
            one_dim = true;
        else if (arg == "-h" || arg == "--help") {
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

    if (ds_reference.n_time != ds_control.n_time || ds_reference.n_time != ds_scenario.n_time)
        Log.warning("The length of the time dimensions of the input files are not equal.");

    // Delta Method needs same length of time dimension for reference and scenario
    if (ds_reference.n_time != ds_scenario.n_time && adjustment_method_name == std::string("delta_method"))
        throw std::runtime_error("Time dimension of reference and scenario input files does not have the same length! This is required for the delta method.");

    // when using -15 to +15 days long-term interval scaling, leap years should not be included and every year must be full.
    if (interval31_scaling && !(ds_reference.n_time % 365 == 0 && ds_control.n_time % 365 == 0 && ds_scenario.n_time % 365 == 0))
        throw std::runtime_error(
            "Data sets should not contain the 29. February and every year must have 365 entries for long-term "
            "31-day interval scaling. Use the \"--monhtly\" flag instead and apply the program on monthly separated data sets.");

    if (get_adjustment_kind() == "add") {
        if (adjustment_method_name == "linear_scaling")
            scaling_func_A = CMethods::Linear_Scaling_add;
        else if (adjustment_method_name == "variance_scaling")
            scaling_func_B = CMethods::Variance_Scaling_add;
        else if (adjustment_method_name == "delta_method")
            scaling_func_A = CMethods::Delta_Method_add;
        else if (adjustment_method_name == "quantile_mapping")
            distribution_func = CMethods::Quantile_Mapping;
        else if (adjustment_method_name == "quantile_delta_mapping")
            distribution_func = CMethods::Quantile_Delta_Mapping;
        else
            throw std::runtime_error("Additive " + adjustment_method_name + " not found!");
    } else if (get_adjustment_kind() == "mult") {
        if (adjustment_method_name == "linear_scaling")
            scaling_func_B = CMethods::Linear_Scaling_mult;
        else if (adjustment_method_name == "delta_method")
            scaling_func_B = CMethods::Delta_Method_mult;
        else if (adjustment_method_name == "quantile_mapping")
            distribution_func = CMethods::Quantile_Mapping;
        else if (adjustment_method_name == "quantile_delta_mapping")
            distribution_func = CMethods::Quantile_Delta_Mapping;
        else
            throw std::runtime_error("Multiplicative " + adjustment_method_name + " not found!");
    } else
        throw std::runtime_error("Unkonwn adjustment kind " + adjustment_kind + "!");
}

template <typename T>
static void stdcout_runtime(T start_time) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end_time - start_time;
    std::cout << ms_double.count() << "ms\n";
}

/**
 * ------------------------------------------------------------
 *   ____                            _        _   _
 *  / ___|___  _ __ ___  _ __  _   _| |_ __ _| |_(_) ___  _ __
 * | |   / _ \| '_ ` _ \| '_ \| | | | __/ _` | __| |/ _ \| '_ \
 * | |__| (_) | | | | | | |_) | |_| | || (_| | |_| | (_) | | | |
 *  \____\___/|_| |_| |_| .__/ \__,_|\__\__,_|\__|_|\___/|_| |_|
 *                      |_|
 */
static void adjust_1d(
    std::vector<float>& v_data_out,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario) {
    if (scaling_func_A != NULL)
        scaling_func_A(v_data_out, v_reference, v_control, v_scenario, interval31_scaling);
    else if (scaling_func_B != NULL)
        scaling_func_B(v_data_out, v_reference, v_control, v_scenario, max_scaling_factor, interval31_scaling);
    else if (distribution_func != NULL)
        distribution_func(v_data_out, v_reference, v_control, v_scenario, adjustment_kind, n_quantiles);
    else
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
            adjust_1d(v_data_out[lat][lon], v_reference_lat_data[lat], v_control_lat_data[lat], v_scenario_lat_data[lat]);
    }
    utils::progress_bar((float)(v_data_out[0].size()), (float)(v_data_out[0].size()));
    std::cout << std::endl;
}

/**
 * ------------------------------------------------------------------------
 *  __  __       _
 * |  \/  | __ _(_)_ __
 * | |\/| |/ _` | | '_ \
 * | |  | | (_| | | | | |
 * |_|  |_|\__,_|_|_| |_|
 */

int main(int argc, char** argv) {
    auto start_time = std::chrono::high_resolution_clock::now();
    utils::show_copyright_notice("BiasAdjustCXX");

    try {
        parse_args(argc, argv);
        Log.info("Data sets loaded");
        Log.info("Method: " + adjustment_method_name + " (" + get_adjustment_kind() + ")");
        if (get_adjustment_kind() == "mult") Log.info("Maximum scaling factor: " + std::to_string(max_scaling_factor));
        for (unsigned i = 0; i < CMethods::scaling_method_names.size(); i++) {
            if (CMethods::scaling_method_names[i] == adjustment_method_name) {
                if (interval31_scaling)
                    Log.info("Scaling will be performed based on long-term 31-day intervals.");
                else
                    Log.info(
                        "Scaling will be performed based on the whole data set. "
                        "The input files should only contain the data for a specific month over the entire period. "
                        "(i.e. this program must be applied to 12 data sets, that contain values only for a specific month over all years.)");

                break;
            }
        }

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
            Log.info("Saving: " + output_filepath + " ...");
            ds_scenario.to_netcdf(output_filepath, variable_name, v_data_out);

        } else {  // adjustment of 3-dimensional data set
            // ? prepare lat x lon x time
            std::vector<std::vector<std::vector<float>>> v_data_out(
                (int)ds_scenario.n_lat,
                std::vector<std::vector<float>>(
                    (int)ds_scenario.n_lon,
                    std::vector<float>(
                        (int)ds_scenario.n_time)));

            Log.info("Starting the adjustment ...");
            adjust_3d(v_data_out);

            Log.info("Preparing data for saving ...");
            std::vector<std::vector<std::vector<float>>>
                v_data_to_save(
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

            Log.info("Saving: " + output_filepath + " ...");
            ds_scenario.to_netcdf(output_filepath, variable_name, v_data_to_save);
        }
        Log.info("Done!");
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
