// -*- lsst-c++ -*-

/**
 * @file NcFileHandler.cpp
 * @brief Class to store, manage and save netCDF datasets
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 * * Description:
 *
 * * Notes:
 *      - Needs to be included in the main program by including "NcFileHandler.h"
 *      - Must be compiled with the main program
 */

/*
 * ----- ----- ----- I N C L U D E ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */

#include "NcFileHandler.h"

#include <fstream>

/*
 * ----- ----- ----- D E F I N I T I O N S ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */

std::string NcFileHandler::time_name = "time";
std::string NcFileHandler::lat_name = "lat";
std::string NcFileHandler::lon_name = "lon";

std::string NcFileHandler::units = "units";
std::string NcFileHandler::lat_unit = "degrees_north";
std::string NcFileHandler::lon_unit = "degrees_east";

/*
 * ----- ----- ----- C L A S S - I M P L E M E N T A T I O N ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */

NcFileHandler::NcFileHandler() {}
NcFileHandler::NcFileHandler(std::string filepath, std::string variable_name) : in_filename(filepath),
                                                                                var_name(variable_name) {
    try {
        std::ifstream ifile;
        ifile.open(in_filename);

        if (!ifile)
            throw std::runtime_error("Could not open file: " + in_filename);
        else
            ifile.close();
        dataFile = new netCDF::NcFile(in_filename, netCDF::NcFile::read);

        time_dim = dataFile->getDim(NcFileHandler::time_name);
        lat_dim = dataFile->getDim(NcFileHandler::lat_name);
        lon_dim = dataFile->getDim(NcFileHandler::lon_name);

        n_lat = lat_dim.getSize();
        n_lon = lon_dim.getSize();
        n_time = time_dim.getSize();

        lat_values = new float[n_lat];
        lon_values = new float[n_lon];
        time_values = new double[n_time];

        // Get the latitude longitude and time values
        lat_var = dataFile->getVar(lat_name);
        if (lat_var.isNull()) throw std::runtime_error("Latitude dimension " + lat_name + " not found!");
        lat_var.getVar(lat_values);

        lon_var = dataFile->getVar(lon_name);
        if (lon_var.isNull()) throw std::runtime_error("Longitude dimension " + lon_name + " not found!");
        lon_var.getVar(lon_values);

        time_var = dataFile->getVar(time_name);
        if (time_var.isNull()) throw std::runtime_error("Time dimension " + time_name + " not found!");
        time_var.getVar(time_values);

        // Get the data of the variable; this is later used to select a specific region. This is kinda open file to reference
        data = dataFile->getVar(var_name);
        if (data.isNull()) throw std::runtime_error("Variable " + var_name + " not found in " + in_filename + "!");

    } catch (netCDF::exceptions::NcException& err) {
        std::cout << err.what() << std::endl;
    }
}
NcFileHandler::~NcFileHandler() {}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Data access management
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

void NcFileHandler::load_whole_dataset(float*** data_out, std::string variable_name) {
    // netCDF::NcVar d = dataFile->getVar(variable_name);
    // d.getVar(*data_out);
}
/** Fills out_arr with timeseries of all location for given latitude (lat).
 * Reading data costs a lot of time so it's better to load multiple locations in one step.
 *
 * @param out_arr output array
 * @param lat latitude of desired locations
 */
void NcFileHandler::fill_lon_timeseries_for_lat(float** out_arr, unsigned lat) {
    std::vector<size_t> startp, countp;
    startp.push_back(0);
    startp.push_back(lat);
    startp.push_back(0);

    countp.push_back(n_time);
    countp.push_back(1);
    countp.push_back(n_lon);

    float tmp[n_time][n_lon];
    data.getVar(startp, countp, *tmp);
    for (unsigned ts = 0; ts < n_time; ts++)
        for (unsigned lon = 0; lon < n_lon; lon++)
            out_arr[lon][ts] = tmp[ts][lon];
}

/** Filles out_arr with all timesteps of one location
 *
 * @param out_arr output array
 * @param lat latitude of desired location
 * @param lon longitude of desired location
 */
void NcFileHandler::fill_timeseries_for_location(float* out_arr, unsigned lat, unsigned lon) {
    std::vector<size_t>
        startp,  // start point
        countp;  // end point
    startp.push_back(0);
    startp.push_back(0);
    startp.push_back(0);

    countp.push_back(1);      // endpoint: startp + 1 -> take only one step
    countp.push_back(n_lat);  // endpoint: all lats
    countp.push_back(n_lon);  // endpoint: all lons

    float tmp[n_lat][n_lon];
    for (unsigned day = 0; day < n_time; day++) {
        startp[0] = day;
        data.getVar(startp, countp, *tmp);
        out_arr[day] = tmp[lat][lon];
    }
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Data saving management
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */
/** Saves a dataset with one variable and only one timestep to file
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param out_data 2d array of data
 */
void NcFileHandler::save_to_netcdf(std::string out_fpath, std::string variable_name, float** out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    // Create netCDF dimensions
    netCDF::NcDim
        out_lat_dim = output_file.addDim(lat_name, n_lat),
        out_lon_dim = output_file.addDim(lon_name, n_lon);

    // Create dimension names
    netCDF::NcVar
        out_lat_var = output_file.addVar(lat_name, netCDF::ncFloat, out_lat_dim),
        out_lon_var = output_file.addVar(lon_name, netCDF::ncFloat, out_lon_dim);

    // Sett attributes
    out_lat_var.putAtt(units, lat_unit);
    out_lon_var.putAtt(units, lon_unit);

    // Define the netCDF variables for the minCorr and location data.
    std::vector<netCDF::NcDim> dim_vector;
    dim_vector.push_back(out_lat_dim);
    dim_vector.push_back(out_lon_dim);

    netCDF::NcVar
        output_var = output_file.addVar(variable_name, netCDF::ncFloat, dim_vector);

    // Write the coordinate variable data to the file.
    out_lat_var.putVar(lat_values);
    out_lon_var.putVar(lon_values);

    std::vector<size_t> countp, startp;
    startp.push_back(0);
    startp.push_back(0);
    startp.push_back(0);
    countp.push_back(n_lat);
    countp.push_back(n_lon);

    float data_to_save[n_lat][n_lon];
    for (unsigned lat = 0; lat < n_lat; lat++)
        for (unsigned lon = 0; lon < n_lon; lon++)
            data_to_save[lat][lon] = out_data[lat][lon];

    // Write data into output file
    output_var.putVar(startp, countp, data_to_save);

    // file closes in destructor of output_var
}

/** Saves a dataset to file (3 dimensions)
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param out_data 3d array of data
 */
void NcFileHandler::save_to_netcdf(std::string out_fpath, std::string variable_name, float*** out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    // Create netCDF dimensions
    netCDF::NcDim
        out_time_dim = output_file.addDim(time_name, n_time),
        out_lat_dim = output_file.addDim(lat_name, n_lat),
        out_lon_dim = output_file.addDim(lon_name, n_lon);

    // Create dimension names
    netCDF::NcVar
        out_time_var = output_file.addVar(time_name, netCDF::ncDouble, out_time_dim),
        out_lat_var = output_file.addVar(lat_name, netCDF::ncFloat, out_lat_dim),
        out_lon_var = output_file.addVar(lon_name, netCDF::ncFloat, out_lon_dim);

    // Sett attributes
    for (std::pair<std::string, netCDF::NcVarAtt> att : time_var.getAtts()) {
        if (att.second.getType().getName() == "char") {
            char value[att.second.getAttLength()];
            att.second.getValues(value);
            out_time_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
        } else if (att.second.getType().getName() == "double") {
            double value[att.second.getAttLength()];
            att.second.getValues(value);
            out_time_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
        }
    }

    for (std::pair<std::string, netCDF::NcVarAtt> att : lon_var.getAtts()) {
        if (att.second.getType().getName() == "char") {
            char value[att.second.getAttLength()];
            att.second.getValues(value);
            out_lon_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
        }
    }

    for (std::pair<std::string, netCDF::NcVarAtt> att : lat_var.getAtts()) {
        if (att.second.getType().getName() == "char") {
            char value[att.second.getAttLength()];
            att.second.getValues(value);
            out_lat_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
        }
    }

    // Define the netCDF variables for the minCorr and location data.
    std::vector<netCDF::NcDim> dim_vector;
    dim_vector.push_back(out_time_dim);
    dim_vector.push_back(out_lat_dim);
    dim_vector.push_back(out_lon_dim);

    netCDF::NcVar
        output_var = output_file.addVar(variable_name, netCDF::ncFloat, dim_vector);

    // Write the coordinate variable data to the file.
    out_time_var.putVar(time_values);
    out_lat_var.putVar(lat_values);
    out_lon_var.putVar(lon_values);

    std::vector<size_t> startp, countp;
    startp.push_back(0);
    startp.push_back(0);
    startp.push_back(0);
    countp.push_back(1);
    countp.push_back(n_lat);
    countp.push_back(n_lon);

    // ? workaround, because putVar does not work with pointer
    // ? -> it works but creates random incostances and destroys the result
    for (size_t time = 0; time < n_time; time++) {
        startp[0] = time;
        float tmp[n_lat][n_lon];
        for (unsigned lat = 0; lat < n_lat; lat++) {
            for (unsigned lon = 0; lon < n_lon; lon++)
                tmp[lat][lon] = out_data[time][lat][lon];
        }
        output_var.putVar(startp, countp, tmp);
    }
    // file closes in destructor of output_var
}

/** Saves a dataset containing multiple variables for only one timestep to file
 *
 * @param out_fpath output file path
 * @param variable_names names of the output variables
 * @param out_data vector of 2d arrays of data
 */
void NcFileHandler::save_to_netcdf(std::string out_fpath, std::vector<std::string> variable_names, std::vector<float**> out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    // Create netCDF dimensions
    netCDF::NcDim
        out_lat_dim = output_file.addDim(lat_name, n_lat),
        out_lon_dim = output_file.addDim(lon_name, n_lon);

    // Create dimension names
    netCDF::NcVar
        out_lat_var = output_file.addVar(lat_name, netCDF::ncFloat, out_lat_dim),
        out_lon_var = output_file.addVar(lon_name, netCDF::ncFloat, out_lon_dim);

    // Write the coordinate variable data to the file.
    out_lat_var.putVar(lat_values);
    out_lon_var.putVar(lon_values);

    // Sett attributes
    for (std::pair<std::string, netCDF::NcVarAtt> att : lon_var.getAtts()) {
        char value[att.second.getAttLength()];
        att.second.getValues(value);
        out_lon_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
    }

    for (std::pair<std::string, netCDF::NcVarAtt> att : lat_var.getAtts()) {
        char* value = new char[att.second.getAttLength()];
        att.second.getValues(value);
        out_lat_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
    }

    // Define the netCDF variables for the minCorr and location data.
    std::vector<netCDF::NcDim> dim_vector;
    dim_vector.push_back(out_lat_dim);
    dim_vector.push_back(out_lon_dim);

    for (unsigned i = 0; i < variable_names.size(); i++) {
        netCDF::NcVar
            output_var = output_file.addVar(variable_names[i], netCDF::ncFloat, dim_vector);

        std::vector<size_t> countp, startp;
        startp.push_back(0);
        startp.push_back(0);
        startp.push_back(0);
        countp.push_back(n_lat);
        countp.push_back(n_lon);

        float data_to_save[n_lat][n_lon];
        for (unsigned lat = 0; lat < n_lat; lat++)
            for (unsigned lon = 0; lon < n_lon; lon++)
                data_to_save[lat][lon] = out_data[i][lat][lon];

        // Write data into output file
        output_var.putVar(startp, countp, data_to_save);
    }
    // file closes in destructor of output_var
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                       TEST
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

void NcFileHandler::test() {
    std::cout << "TEST" << std::endl;
}

/*
 * ----- ----- ----- E O F ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- |
 */
