// -*- lsst-c++ -*-

/**
 * @file Manager.cxx
 * @brief Manager class which controls the program flow of BiasAdjustCXX
 * @author Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
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
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Includes
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include "Manager.hxx"

#include <algorithm>
#include <future>
#include <iostream>
#include <vector>

#include "CMethods.hxx"
#include "Utils.hxx"
#include "colors.h"

/**
 * Constructor which parses a string containing arguments
 * that are passed to the `main` function while executing the BiasAdjustCXX
 * program. This class handles the program flow and is only
 * instantiate once within the `main` function of `main.cxx`.
 *
 * @param argc number of arguments
 * @param argv qrgv passed through main function
 */
Manager::Manager(int argc, char** argv) : argc(argc),
                                          argv(argv),
                                          variable_name(""),
                                          output_filepath(""),
                                          adjustment_method_name(""),
                                          adjustment_settings(AdjustmentSettings()),
                                          one_dim(false),
                                          n_jobs(1),
                                          adjustment_function(NULL),
                                          log(utils::Log()) {
    parse_args();
}

Manager::~Manager() {
    delete ds_reference;
    delete ds_control;
    delete ds_scenario;
}

/**
 * Prints information about the adjustment which is called within
 * this function. After the adjustment a bias corrected data set
 * will be saved.
 */
void Manager::run_adjustment() {
    log.info("Data sets available");
    log.info("Method: " + adjustment_method_name + " (" + get_adjustment_kind() + ")");
    log.info("Threads: " + std::to_string(n_jobs));
    if (get_adjustment_kind() == "mult") log.info("Maximum scaling factor: " + std::to_string(adjustment_settings.max_scaling_factor));
    for (unsigned i = 0; i < CMethods::scaling_method_names.size(); i++) {
        if (CMethods::scaling_method_names[i] == adjustment_method_name) {
            if (adjustment_settings.interval31_scaling)
                log.info("Scaling will be performed based on long-term 31-day intervals.");
            else
                log.info(
                    "Scaling will be performed based on the whole data set. "
                    "The input files should only contain the data for a specific month over the entire period. "
                    "(i.e. this program must be applied to 12 data sets, that contain values only for a specific month over all years.)"
                );
            break;
        }
    }

    if (one_dim) {  // adjustment of data set containing only one grid cell
        std::vector<float>
            v_data_out((int)ds_scenario->n_time),
            v_reference((int)ds_reference->n_time),
            v_control((int)ds_control->n_time),
            v_scenario((int)ds_scenario->n_time);

        ds_reference->get_timeseries(v_reference);
        ds_control->get_timeseries(v_control);
        ds_scenario->get_timeseries(v_scenario);

        adjust_1d(v_data_out, v_reference, v_control, v_scenario);
        log.info("Adjustment done!");
        log.info("Saving: " + output_filepath + " ...");
        ds_scenario->to_netcdf(output_filepath, variable_name, v_data_out);

    } else {  // adjustment of 3-dimensional data set
        // ? prepare result vector lat x lon x time
        std::vector<std::vector<std::vector<float>>> v_data_out(
            (int)ds_scenario->n_lat,
            std::vector<std::vector<float>>(
                (int)ds_scenario->n_lon,
                std::vector<float>((int)ds_scenario->n_time)
            )
        );

        log.info("Starting the adjustment ...");
        adjust_3d(v_data_out);

        log.info("Preparing data for saving ...");
        std::vector<std::vector<std::vector<float>>>
            v_data_to_save(
                (int)ds_scenario->n_time,
                std::vector<std::vector<float>>(
                    (int)ds_scenario->n_lat,
                    std::vector<float>((int)ds_scenario->n_lon)
                )
            );

        // ? reshape to time x lat x lon
        for (unsigned lat = 0; lat < v_data_out.size(); lat++)
            for (unsigned lon = 0; lon < v_data_out[lat].size(); lon++)
                for (unsigned time = 0; time < v_data_out[lat][lon].size(); time++)
                    v_data_to_save[time][lat][lon] = v_data_out[lat][lon][time];

        log.info("Saving: " + output_filepath);
        ds_scenario->to_netcdf(output_filepath, variable_name, v_data_to_save);
    }
    log.info("Done!");
}

/**
 * Command-line output of the program usage hints.
 */
void Manager::show_usage() {
    std::cout << BOLDBLUE << "Usage: " RESET << "BiasAdjustCXX"
              << "\t\t\t\\\n"
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
              << GREEN << "\t    --no-group\t\t\t" << RESET << "disables the adjustment based on long-term 31-day intervals for the sclaing-based methods; "
                                                               "mean calculation will be performed on the whole data set\n"
              << GREEN << "\t    --max-scaling-factor\t" << RESET << "define the maximum scaling factor to avoid unrealistic results when adjusting ratio based variables "
                                                                     "(only for scaling methods; default: 10)\n"
              << GREEN << "\t-p, --n_processes\t\t\t" << RESET << "Number of threads to start (only for 3-dimensional adjustments; default: 1)"
              << "\n\n"
              << BOLDBLUE << "Requirements: \n"
              << RESET
              << "-> data sets must have the file type NetCDF\n"
              << "-> for scaling-based adjustments: all input files must have 365 days per year (no February 29th.) otherwise the " << GREEN << "--no-group" << RESET << " flag is needed (see notes section below)\n"
              << "-> all data must be in format: [time][lat][lon] (if " << GREEN << "--1dim" << RESET << " is not slected) and values of type float\n"
              << "-> latitudes, longitudes and times must be named 'lat', 'lon' and 'time'\n"
              << RESET << std::endl;

    std::cout << BOLDBLUE << "Available methods: " << RESET << "\n-> ";
    std::vector<std::string> all_methods;
    all_methods.reserve(CMethods::scaling_method_names.size() + CMethods::distribution_method_names.size());  // preallocate memory
    all_methods.insert(all_methods.end(), CMethods::scaling_method_names.begin(), CMethods::scaling_method_names.end());
    all_methods.insert(all_methods.end(), CMethods::distribution_method_names.begin(), CMethods::distribution_method_names.end());

    for (size_t i = 0; i < all_methods.size(); i++) std::cout << all_methods[i] << " ";
    std::cout << std::endl;
    std::cout << YELLOW << "\nNotes: " << RESET
              << "\n- When not using the " << GREEN << "--no-group" << RESET << " flag it is required that all input files must have 365 days per year (no February 29th.) "
              << "The Linear Scaling, Variance Scaling and Delta Method need a wrapper script when the " << GREEN << "--no-group" << RESET << " flag is used to apply this program on for example monthly separated files i.e. "
              << "to adjust 30 years of data, all input files need to be separated into 12 groups, one group for each month, than this program can be applied to every long-term month."
              << "\n\n- The Delta Method requires that the time series of the control period have the same length as the time series to be adjusted.";

    std::cout << YELLOW << "\n\n====== References ======" << RESET
              << "\n- Copyright (C) Benjamin Thomas Schwertfeger (2023) development@b-schwertfeger.de"
              << "\n- Unidata's NetCDF Programming Interface NetCDFCxx Data structures: http://doi.org/10.5065/D6H70CW6"
              << "\n- Mathematical foundations:"
              << "\n(1) Beyer, R., Krapp, M., and Manica, A.: An empirical evaluation of bias correction methods for palaeoclimate simulations, Climate of the Past, 16, 1493–1508, https://doi.org/10.5194/cp-16-1493-2020, 2020"
              << "\n\n(2) Cannon, A. J., Sobie, S. R., and Murdock, T. Q.: Bias Correction of GCM Precipitation by Quantile Mapping: How Well Do Methods Preserve Changes in Quantiles and Extremes?, Journal of Climate, 28, 6938 – 6959, https://doi.org/10.1175/JCLI-D-14-00754.1, 2015."
              << "\n\n(3) Maraun, D.: Nonstationarities of Regional Climate Model Biases in European Seasonal Mean Temperature and Precipitation Sums, Geophysical Research Letters, 39, 6706–, https://doi.org/10.1029/2012GL051210, 2012."
              << "\n\n(4) Teutschbein, C. and Seibert, J.: Bias correction of regional climate model simulations for hydrological climate-change impact studies: Review and evaluation of different methods, Journal of Hydrology, s 456–457, 12–29, https://doi.org/10.1016/j.jhydrol.2012.05.052, 2012."
              << "\n\n(5) Tong, Y., Gao, X., Han, Z., Xu, Y., Xu, Y., and Giorgi, F.: Bias correction of temperature and precipitation over China for RCM simulations using the QM and QDM methods, Climate Dynamics, 57, https://doi.org/10.1007/s00382-020-05447-4, 2021."
              << std::endl;
    std::cout.flush();
}

/**
 * Parses the arguments that were passed to the constructor.
 * This includes loading the input data sets, checking if
 * the respective adjustment conditions met and sets
 * the adjustment relevant variables.
 */
void Manager::parse_args() {
    if (argc == 1) {
        show_usage();
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
                adjustment_settings.n_quantiles = (unsigned)std::stoi(argv[++i]);
        } else if (arg == "-m" || arg == "--method") {
            if (i + 1 < argc)
                adjustment_method_name = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "-k" || arg == "--kind") {
            if (i + 1 < argc) {
                adjustment_settings.kind = argv[++i];
                adjustment_settings.kind = get_adjustment_kind();

            } else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--max-scaling-factor") {
            if (i + 1 < argc)
                adjustment_settings.max_scaling_factor = std::stoi(argv[++i]);
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--no-group")
            adjustment_settings.interval31_scaling = false;
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc)
                output_filepath = argv[++i];
            else
                throw std::runtime_error(arg + " requires one argument!");
        } else if (arg == "--1dim")
            one_dim = true;
        else if (arg == "-p" || arg == "--n_processes") {
            n_jobs = std::stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            show_usage();
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
            log.warning("Unknown argument: " + arg + "!");
    }

    /* Requirements/Conditions for the whole adjustment procedure */
    if (variable_name.empty()) throw std::runtime_error("No variable name defined!");
    if (reference_fpath.empty()) throw std::runtime_error("No reference file defined!");
    if (control_fpath.empty()) throw std::runtime_error("No control file defined!");
    if (scenario_fpath.empty()) throw std::runtime_error("No scenario file defined!");
    if (output_filepath.empty()) throw std::runtime_error("No output file defined!");
    if (adjustment_method_name.empty()) throw std::runtime_error("No method specified!");
    if (adjustment_settings.kind.empty()) throw std::runtime_error("Adjustmend kind is empty!");

    // 1- or 3-dimensional adjustment
    if (one_dim) {
        ds_reference = new NcFileHandler(reference_fpath, variable_name, 1);
        ds_control = new NcFileHandler(control_fpath, variable_name, 1);
        ds_scenario = new NcFileHandler(scenario_fpath, variable_name, 1);
    } else {
        ds_reference = new NcFileHandler(reference_fpath, variable_name, 3);
        ds_control = new NcFileHandler(control_fpath, variable_name, 3);
        ds_scenario = new NcFileHandler(scenario_fpath, variable_name, 3);

        // latitudes and longitudes must have the same lengths in all input files
        if (ds_reference->n_lat != ds_control->n_lat || ds_reference->n_lat != ds_scenario->n_lat)
            throw std::runtime_error("Input files have unqual lengths of the `lat` (latitude) dimension.");
        else if (ds_reference->n_lon != ds_control->n_lon || ds_reference->n_lon != ds_scenario->n_lon)
            throw std::runtime_error("Input files have unqual lengths of the `lon` (longitude) dimension.");
    }

    // Time dimensions can have different lenghts but it is not recommanded
    if (ds_reference->n_time != ds_control->n_time || ds_reference->n_time != ds_scenario->n_time)
        log.warning("Input files have different sizes for the time dimension.");

    // Delta Method needs same length of time dimension for reference and scenario
    if (ds_reference->n_time != ds_scenario->n_time && adjustment_method_name == std::string("delta_method"))
        throw std::runtime_error(
            "Time dimension of reference and scenario input files "
            "does not have the same length! This is required for the delta method."
        );

    // When using long-term 31-day interval scaling, leap years should not be included and every year must be complete.
    if (adjustment_settings.interval31_scaling &&
        !(ds_reference->n_time % 365 == 0 &&
          ds_control->n_time % 365 == 0 &&
          ds_scenario->n_time % 365 == 0))
        throw std::runtime_error(
            "Data sets should not contain the 29. February and every year must have 365 entries for long-term "
            "31-day interval scaling. Use the '--no-group' flag to adjust the data set without any moving window."
        );

    // misc
    if (n_jobs != 1 && one_dim) log.warning("Using only one thread because of the adjustment of a 1-dimensional data set.");

    // setting the method
    if (adjustment_settings.kind == "add" || adjustment_settings.kind == "mult") {
        if (adjustment_method_name == "linear_scaling")
            adjustment_function = CMethods::Linear_Scaling;
        else if (adjustment_method_name == "variance_scaling") {
            if (adjustment_settings.kind != "mult")
                adjustment_function = CMethods::Variance_Scaling;
            else
                throw std::runtime_error("Multiplicative Variance Scaling not available!");
        } else if (adjustment_method_name == "delta_method")
            adjustment_function = CMethods::Delta_Method;
        else if (adjustment_method_name == "quantile_mapping")
            adjustment_function = CMethods::Quantile_Mapping;
        else if (adjustment_method_name == "quantile_delta_mapping")
            adjustment_function = CMethods::Quantile_Delta_Mapping;
        else
            throw std::runtime_error("Method " + adjustment_method_name + "(" + adjustment_settings.kind + ") not found!");
    } else
        throw std::runtime_error("Unkonwn adjustment kind " + adjustment_settings.kind + "!");
}

/**
 * Returns the selected adjustment kind
 *
 * @return adjustment kind, additive ("add" || "+") or multiplicative ("mult" || "*")
 */
std::string Manager::get_adjustment_kind() {
    return (adjustment_settings.kind == "additive" ||
            adjustment_settings.kind == "add" ||
            adjustment_settings.kind == "+")
               ? "add"
           : (adjustment_settings.kind == "multiplicative" ||
              adjustment_settings.kind == "mult" ||
              adjustment_settings.kind == "*")
               ? "mult"
               : "";
}

/**
 * Invokes the selected bias adjustment function (that is not NULL)
 * to adjust the 1-dimensional time series v_scenario
 *
 * @param v_data_out 1D vector to store the adjusted result/time series in
 * @param v_reference 1D reference data (control period)
 * @param v_control 1D modeled data (control period)
 * @param v_scenario 1D data to adjust (scenario period)
 */
void Manager::adjust_1d(
    std::vector<float>& v_data_out,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario
) {
    if (adjustment_function != NULL)
        adjustment_function(v_data_out, v_reference, v_control, v_scenario, adjustment_settings);
    else
        throw std::runtime_error("Unknown adjustment method " + adjustment_method_name + " (" + adjustment_settings.kind + ")!");
}

/**
 * Handles the adjustment of a 3-dimensional data set by loading the input data
 * per latitude
 * -> All longitudes per latitude are adjusted in one iteration, this is because
 *    loading all time series at once would crash the most systems and loading
 *    every time series alone takes too much time.
 *
 * @param v_data_out 3D vector that is used to store the bias adjusted results
 */
void Manager::adjust_3d(std::vector<std::vector<std::vector<float>>>& v_data_out) {
    std::vector<std::future<void>> tasks;
    for (unsigned lon = 0; lon < ds_scenario->n_lon; lon++) {
        std::vector<std::vector<float>> v_reference_lat_data(
            (int)ds_reference->n_lat,
            std::vector<float>((int)ds_reference->n_time)
        );

        std::vector<std::vector<float>> v_control_lat_data(
            (int)ds_control->n_lat,
            std::vector<float>((int)ds_control->n_time)
        );

        std::vector<std::vector<float>> v_scenario_lat_data(
            (int)ds_scenario->n_lat,
            std::vector<float>((int)ds_scenario->n_time)
        );

        ds_reference->get_lat_timeseries_for_lon(v_reference_lat_data, lon);
        ds_control->get_lat_timeseries_for_lon(v_control_lat_data, lon);
        ds_scenario->get_lat_timeseries_for_lon(v_scenario_lat_data, lon);

        for (unsigned lat = 0; lat < ds_scenario->n_lat; lat++) {
            tasks.push_back(std::async(
                &Manager::adjust_1d,
                this,
                std::ref(v_data_out[lat][lon]),
                std::ref(v_reference_lat_data[lat]),
                std::ref(v_control_lat_data[lat]),
                std::ref(v_scenario_lat_data[lat])
            ));

            if (lat % n_jobs == 0 || ds_scenario->n_lat - 1 == lat) {
                for (auto& e : tasks) e.get();
                tasks.clear();
            }
        }
        utils::progress_bar((float)lon, (float)(v_data_out[0].size()));
    }
    utils::progress_bar((float)(v_data_out[0].size()), (float)(v_data_out[0].size()));
    std::cout << std::endl;
}
