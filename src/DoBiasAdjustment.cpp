
// -*- lsst-c++ -*-

/**
 * @file DoBiasAdjustment
 * @brief Main program to bias adjust climate data
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 *
 * * Description
 *      Main program to bias adjust climate data;
 *      - Loading datasets
 *      - Iteration over all cells
 *      - Application of the selected adjustment
 *      - Save to file after adjustment
 * * Compilaition
 *    g++ -std=c++17 \
 *        DoBiasAdjustment.cpp \
 *        -o DoBiasAdjustment \
 *        ../lib/Utils.cpp \
 *        ../lib/MyMath.cpp \
 *        ../lib/CMethods.cpp \
 *        ../lib/NCFileHandler.cpp \
 *        -lnetcdf-cxx4
 *
 */

/*
 * ----- ----- ----- I N C L U D E ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include <math.h>

#include "../lib/CMethods.h"
#include "../lib/NcFileHandler.h"
#include "../lib/Utils.h"
#include "../lib/colors.h"
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

static unsigned n_quantiles = 100;

static utils::Log Log = utils::Log();

/*
 * ----- ----- ----- P R O G R A M - M A N A G E M E N T ----- ----- ----- ----- ----- ----- ----- ----- ----- */

static bool isInStrV(std::vector<std::string> v, std::string string) {
    if (std::find(v.begin(), v.end(), string) != v.end())
        return true;
    else
        return false;
}

static void show_usage(std::string name) {
    std::cerr << BOLDBLUE << "Usage: " RESET << name << "\t\t\\\n"
              << GREEN << "\t --ref " << RESET << "observation_data.nc\t\\\n"
              << GREEN << "\t --contr " << RESET << "control_data.nc\t\\\n"
              << GREEN << "\t --scen " << RESET << "data_to_adjust.nc\t\\\n"
              << GREEN << "\t -v " << RESET << "tas\t\t\t\t\\\n"
              << GREEN << "\t -m " << RESET << "linear_scaling\n\n"
              << BOLDBLUE << "Parameters:\n"
              << RESET
              << "    required:\n"
              << GREEN << "\t--ref, --reference\t" << RESET << "Observation / Reanalysis => Inputfile or Filepath\n"
              << GREEN << "\t--contr, --control\t" << RESET << "Control period => Inputfile or Filepath\n"
              << GREEN << "\t--scen, --scenario\t" << RESET << "Scenario / data to adjust => Inputfile or Filepath\n"
              << GREEN << "\t-o, --output\t\t" << RESET << "Outputfile / Filepath\n"
              << GREEN << "\t-v, --variable\t\t" << RESET << "Variablename (e.g.: tas, tsurf, pr) \n"
              << "    optional:\n"
              << GREEN << "\t-h, --help\t\t" << RESET << "Show this help message\n"
              << GREEN << "\t-k, --kind\t\t" << RESET << "Kind of adjustment (e.g. '+' or '*' for additive or multiplicative method)\n"
              << GREEN << "\t-q, --quantiles\t\t" << RESET << "Number of quantiles to use when using a quantile adjustment\n"
              << "\n\n"
              << BOLDBLUE << "Requirements: \n"
              << RESET
              << "-> Data sets must be filetype NetCDF\n"
              << "-> All data must be in format: [time][lat][lon] and values type float\n"
              << "-> Latitudes and longitudes must be named 'lat' and 'lon', Time = 'time'\n"
              << RESET << std::endl;

    std::cerr << BOLDBLUE << "Available methods: " << RESET << "\n-> ";
    std::vector<std::string> all_methods;
    all_methods.reserve(CMethods::simple_method_names.size() + CMethods::quantile_method_names.size());  // preallocate memory
    all_methods.insert(all_methods.end(), CMethods::simple_method_names.begin(), CMethods::simple_method_names.end());
    all_methods.insert(all_methods.end(), CMethods::quantile_method_names.begin(), CMethods::quantile_method_names.end());

    for (size_t i = 0; i < all_methods.size(); i++) std::cerr << all_methods[i] << " ";
    std::cout << std::endl;
    std::cerr << YELLOW << "\nNotes: " << RESET
              << "\n- Linear Scaling, Variance Scaling and Delta method need a wrapper script to apply this program on monthly separated files i.e. "
              << "you want to adjust 30 years of data, you have to separate all inputfiles in 12 groups, one group for each month and then you apply this "
              << "program on every individual monthly separated data set.";
    std::cout.flush();
}

static int parse_args(int argc, char** argv) {
    if (argc == 1) {
        show_usage(argv[0]);
        return 1;
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
            else {
                Log.error(arg + " requires one argument!");
                return 1;
            }
        } else if (arg == "--contr" || arg == "--control") {
            if (i + 1 < argc)
                control_fpath = argv[++i];
            else {
                Log.error(arg + " requires one argument!");
                return 1;
            }
        } else if (arg == "--scen" || arg == "--scenario") {
            if (i + 1 < argc)
                scenario_fpath = argv[++i];
            else {
                Log.error(arg + " requires one argument!");
                return 1;
            }
        } else if (arg == "-v" || arg == "--variable") {
            if (i + 1 < argc)
                variable_name = argv[++i];
            else {
                Log.error(arg + " requires one argument!");
                return 1;
            }
        } else if (arg == "-q" || arg == "--quantiles") {
            if (i + 1 < argc)
                n_quantiles = (unsigned)std::stoi(argv[++i]);
        } else if (arg == "-m" || arg == "--method") {
            if (i + 1 < argc)
                adjustment_method_name = argv[++i];
            else {
                Log.error(arg + " requires one argument!");
                return 1;
            }
        } else if (arg == "k" || arg == "--kind") {
            if (i + 1 < argc)
                adjustment_kind = argv[++i];
            else {
                Log.error(arg + " requires one argument!");
                return 1;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc)
                output_filepath = argv[++i];
            else {
                Log.error(arg + " requires one argument!");
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            show_usage(argv[0]);
            return 1;
        } else
            Log.warning("Unknown argument " + arg + "!");
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
        ds_reference = NcFileHandler(reference_fpath, variable_name),
        ds_control = NcFileHandler(control_fpath, variable_name),
        ds_scenario = NcFileHandler(scenario_fpath, variable_name);
    }

    if (ds_reference.n_lat != ds_control.n_lat || ds_reference.n_lat != ds_scenario.n_lat)
        std::runtime_error("Latitude dimension of input files does not have the same length!");
    else if (ds_reference.n_lon != ds_control.n_lon || ds_reference.n_lon != ds_scenario.n_lon)
        std::runtime_error("Longitude dimension of input files does not have the same length!");
    else if (ds_reference.n_time != ds_control.n_time || ds_reference.n_time != ds_scenario.n_time)
        std::runtime_error("Time dimension input files does not have the same length!");

    return 0;
}

/*
 * ----- ----- ----- C O M P U T A T I O N ----- ----- ----- ----- ----- ----- ----- ----- ----- */

static void do_simple_adjustment(float*** data_out) {
    CM_Func_ptr_simple apply_adjustment = CMethods::get_cmethod_simple(adjustment_method_name);

    for (unsigned lat = 0; lat < ds_reference.n_lat; lat++) {
        // ? Fill whole row of data for each dataset
        float** reference_lon_data = new float*[ds_reference.n_lon];
        float** control_lon_data = new float*[ds_control.n_lon];
        float** scenario_lon_data = new float*[ds_scenario.n_lon];

        for (unsigned i = 0; i < ds_reference.n_lon; i++) {
            reference_lon_data[i] = new float[ds_reference.n_time];
            control_lon_data[i] = new float[ds_control.n_time];
            scenario_lon_data[i] = new float[ds_scenario.n_time];
        }

        ds_reference.fill_lon_timeseries_for_lat(reference_lon_data, lat);
        ds_control.fill_lon_timeseries_for_lat(control_lon_data, lat);
        ds_scenario.fill_lon_timeseries_for_lat(scenario_lon_data, lat);

        for (unsigned lon = 0; lon < ds_reference.n_lon; lon++)
            apply_adjustment(
                data_out[lat][lon],
                reference_lon_data[lon],
                control_lon_data[lon],
                scenario_lon_data[lon],
                ds_reference.n_time,
                adjustment_kind);

        utils::progress_bar((float)lat, (float)ds_reference.n_lat);
    }
    utils::progress_bar((float)ds_reference.n_lat, (float)ds_reference.n_lat);
}

static void do_quantile_adjustment(float*** data_out) {
    CM_Func_ptr_quantile apply_adjustment = CMethods::get_cmethod_quantile(adjustment_method_name);

    for (unsigned lat = 0; lat < ds_reference.n_lat; lat++) {
        float** reference_lon_data = new float*[ds_reference.n_lon];
        float** control_lon_data = new float*[ds_control.n_lon];
        float** scenario_lon_data = new float*[ds_scenario.n_lon];

        for (unsigned i = 0; i < ds_reference.n_lon; i++) {
            reference_lon_data[i] = new float[ds_reference.n_time];
            control_lon_data[i] = new float[ds_control.n_time];
            scenario_lon_data[i] = new float[ds_scenario.n_time];
        }

        // ? Fill whole row of data for each dataset
        ds_reference.fill_lon_timeseries_for_lat(reference_lon_data, lat);
        ds_control.fill_lon_timeseries_for_lat(control_lon_data, lat);
        ds_scenario.fill_lon_timeseries_for_lat(scenario_lon_data, lat);

        for (unsigned lon = 0; lon < ds_reference.n_lon; lon++) {
            apply_adjustment(
                data_out[lat][lon],
                reference_lon_data[lon],
                control_lon_data[lon],
                scenario_lon_data[lon],
                ds_reference.n_time,
                adjustment_kind,
                n_quantiles);
        }

        for (unsigned xlat = 0; xlat < ds_reference.n_lat; xlat++) {
            delete reference_lon_data[xlat];
            delete control_lon_data[xlat];
            delete scenario_lon_data[xlat];
        }
        delete[] reference_lon_data;
        delete[] control_lon_data;
        delete[] scenario_lon_data;

        utils::progress_bar((float)lat, (float)(ds_reference.n_lat - 1));
    }
    utils::progress_bar((float)ds_reference.n_lat, (float)(ds_reference.n_lat - 1));
}

static void do_adjustment(float*** data_out) {
    if (isInStrV(CMethods::simple_method_names, adjustment_method_name))
        do_simple_adjustment(data_out);
    if (isInStrV(CMethods::quantile_method_names, adjustment_method_name))
        do_quantile_adjustment(data_out);
    else
        std::runtime_error("Unknown adjustment method " + adjustment_method_name + "!");
    std::cout << std::endl;
}

/*
 * ----- ----- ----- M A I N ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

int main(int argc, char** argv) {
    // ? parse args and open datasets
    int args_result = parse_args(argc, argv);
    if (args_result != 0) return args_result;

    // ? prepare memory lat x lon x time
    float*** data_out = new float**[(int)ds_reference.n_lat];
    for (unsigned lat = 0; lat < (int)ds_reference.n_lat; lat++) {
        data_out[lat] = new float*[(int)ds_reference.n_lon];
        for (unsigned lon = 0; lon < (int)ds_reference.n_lon; lon++)
            data_out[lat][lon] = new float[(int)ds_reference.n_time];
    }

    // ? apply adjustment
    do_adjustment(data_out);

    // ? reshape to lat x lon x time
    float*** data_to_save = new float**[ds_scenario.n_time];
    for (unsigned time = 0; time < ds_scenario.n_time; time++) {
        data_to_save[time] = new float*[ds_scenario.n_lat];
        for (unsigned lat = 0; lat < ds_scenario.n_lat; lat++) {
            data_to_save[time][lat] = new float[ds_scenario.n_lon];
            for (unsigned lon = 0; lon < ds_scenario.n_lon; lon++)
                data_to_save[time][lat][lon] = data_out[lat][lon][time];
        }
    }

    Log.info("Saving " + output_filepath);
    ds_scenario.save_to_netcdf(output_filepath, variable_name, data_to_save);
    Log.info("SUCCESS!");

    return 0;
}

/*
 * ----- ----- ----- E O F ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */
