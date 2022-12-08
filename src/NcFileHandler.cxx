// -*- lsst-c++ -*-

/**
 * @file NcFileHandler.cxx
 * @brief Class to store, manage and save netCDF datasets
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 *
 *  * Copyright (C) 2022 Benjamin Thomas Schwertfeger
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
 */

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Includes and Namespaces
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include "NcFileHandler.hxx"

#include <fstream>

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Definitions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

std::string NcFileHandler::time_name = "time";
std::string NcFileHandler::lat_name = "lat";
std::string NcFileHandler::lon_name = "lon";

std::string NcFileHandler::units = "units";
std::string NcFileHandler::lat_unit = "degrees_north";
std::string NcFileHandler::lon_unit = "degrees_east";

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Class Implementation
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * Creates empty NcFileHandler class that handles no file
 *
 * Can be used to save time series without any attributes
 */
NcFileHandler::NcFileHandler() : handles_file(false) {}

/**
 * Creates the NcFileHandler class
 * -> Load the NetCDF-based file `filepath` by given `variable_name` and `n_dimensions`
 *
 * @param filepath path to file that should be loaded
 * @param variable_name variable to load into this class (only one variable per NcFileHandler instance)
 * @param n_dimensions number of dimensions of this variable within the data set. Only 1 or 3 is valid.
 */
NcFileHandler::NcFileHandler(std::string filepath, std::string variable_name, unsigned n_dimensions) : in_filename(filepath),
                                                                                                       var_name(variable_name),
                                                                                                       handles_file(true),
                                                                                                       n_dimensions(n_dimensions) {
    try {
        std::ifstream ifile;
        ifile.open(in_filename);

        if (!ifile)
            throw std::runtime_error("Could not open file: " + in_filename);
        else
            ifile.close();
        dataFile = new netCDF::NcFile(in_filename, netCDF::NcFile::read);

        time_dim = dataFile->getDim(NcFileHandler::time_name);
        n_time = time_dim.getSize();
        time_values = new double[n_time];

        time_var = dataFile->getVar(time_name);
        if (time_var.isNull()) throw std::runtime_error("Time dimension <" + NcFileHandler::time_name + "> not found!");
        time_var.getVar(time_values);

        if (n_dimensions == 3) {
            lat_dim = dataFile->getDim(NcFileHandler::lat_name);
            n_lat = lat_dim.getSize();
            lat_values = new float[n_lat];

            lat_var = dataFile->getVar(lat_name);
            if (lat_var.isNull()) throw std::runtime_error("Latitude dimension <" + NcFileHandler::lat_name + "> not found!");
            lat_var.getVar(lat_values);

            lon_dim = dataFile->getDim(NcFileHandler::lon_name);
            n_lon = lon_dim.getSize();
            lon_values = new float[n_lon];

            lon_var = dataFile->getVar(NcFileHandler::lon_name);
            if (lon_var.isNull()) throw std::runtime_error("Longitude dimension <" + lon_name + "> not found!");
            lon_var.getVar(lon_values);
        } else if (n_dimensions != 1)
            throw std::runtime_error("Only 1 and 3-dimensional data sets are supported!");

        // Get the data of the variable; this is later used to select a specific region. This is kinda open file to reference
        data = dataFile->getVar(var_name);
        if (data.isNull()) throw std::runtime_error("Variable <" + var_name + "> not found in " + in_filename + "!");

    } catch (netCDF::exceptions::NcException& err) {
        std::cout << err.what() << std::endl;
        exit(1);
    }
}

/**
 * mandatory
 */
NcFileHandler::~NcFileHandler() {
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Data access management
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/** Fills the 2D vector `v_out_arr` with time series of all latitudes for given longitude (`lon`).
 *  -> NcFileHandler must hold 3-dimensional data
 *
 * @param v_out_arr 2D output vector
 * @param lon longitude of desired locations
 */
void NcFileHandler::get_lat_timeseries_for_lon(std::vector<std::vector<float>>& v_out_arr, unsigned lon) {
    std::vector<size_t> startp, countp;
    startp.push_back(0);
    startp.push_back(0);
    startp.push_back(lon);

    countp.push_back(n_time);
    countp.push_back(n_lat);
    countp.push_back(1);

    float tmp[n_time][n_lat];
    this->data.getVar(startp, countp, *tmp);
    for (unsigned ts = 0; ts < n_time; ts++)
        for (unsigned lat = 0; lat < n_lat; lat++)
            v_out_arr[lat][ts] = tmp[ts][lat];
}

/** Fills the 2D vector `v_out_arr` with all timesteps of one location of 3-dimensional data set based on longitude (`lon`)
 *  -> NcFileHandler must hold 3-dimensional data
 *
 * @param v_out_arr output array
 * @param lat latitude of desired location
 * @param lon longitude of desired location
 */
void NcFileHandler::get_timeseries(std::vector<float>& v_out_arr, unsigned lat, unsigned lon) {
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
        v_out_arr[day] = tmp[lat][lon];
    }
}

/** Fills the 1D vector `v_out_arr` with all timesteps of one location of 1-dimensional data set
 *  -> NcFileHandler must hold a 1-dimensional data
 *
 * @param v_out_arr output array
 */
void NcFileHandler::get_timeseries(std::vector<float>& v_out_arr) {
    std::vector<size_t>
        startp,  // start point
        countp;  // end point
    startp.push_back(0);
    countp.push_back(n_time);  // endpoint: startp + 1 -> take only one step

    float tmp[n_time];
    data.getVar(startp, countp, tmp);

    for (unsigned day = 0; day < n_time; day++) v_out_arr[day] = tmp[day];
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Data saving management
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/** Saves a data set with one variable for one time dimension (1D vector)
 *  -> time dimension gets same attributes as handled file
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param out_data 1D array of data to save
 */
void NcFileHandler::to_netcdf(std::string out_fpath, std::string variable_name, float* out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    // ? Save vector with time attributes from handled file
    netCDF::NcDim out_time_dim = output_file.addDim(time_name, n_time);
    netCDF::NcVar out_time_var = output_file.addVar(time_name, netCDF::ncDouble, out_time_dim);

    // Set attributes
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

    std::vector<netCDF::NcDim> dim_vector;
    dim_vector.push_back(out_time_dim);

    netCDF::NcVar output_var = output_file.addVar(variable_name, netCDF::ncFloat, dim_vector);
    out_time_var.putVar(time_values);

    std::vector<size_t> startp, countp;
    startp.push_back(0);
    countp.push_back(n_time);

    output_var.putVar(startp, countp, out_data);
}

/** Saves a data set with one variable for one time dimension (1D vector)
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param v_out_data 1D array of data to save
 */
void NcFileHandler::to_netcdf(std::string out_fpath, std::string variable_name, std::vector<float>& v_out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    if (this->handles_file) {
        netCDF::NcDim out_time_dim = output_file.addDim(time_name, n_time);
        netCDF::NcVar out_time_var = output_file.addVar(time_name, netCDF::ncDouble, out_time_dim);
        // Set attributes
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

        std::vector<netCDF::NcDim> dim_vector;
        dim_vector.push_back(out_time_dim);

        netCDF::NcVar output_var = output_file.addVar(variable_name, netCDF::ncFloat, dim_vector);
        out_time_var.putVar(time_values);

        std::vector<size_t> startp, countp;
        startp.push_back(0);
        countp.push_back(n_time);
        output_var.putVar(startp, countp, &v_out_data[0]);

    } else {
        netCDF::NcDim out_time_dim = output_file.addDim("time", v_out_data.size());
        netCDF::NcVar out_time_var = output_file.addVar(time_name, netCDF::ncDouble, out_time_dim);

        std::vector<netCDF::NcDim> dimensions;
        dimensions.push_back(out_time_dim);

        netCDF::NcVar output_var = output_file.addVar(variable_name, netCDF::ncFloat, dimensions);
        std::vector<size_t> startp, countp;
        startp.push_back(0);
        countp.push_back(v_out_data.size());
        output_var.putVar(startp, countp, &v_out_data[0]);
    }
}

/** Saves a 2-dimensional data set with one variable for one timestep to file
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param out_data 2D array of data to save
 */
void NcFileHandler::to_netcdf(std::string out_fpath, std::string variable_name, float** out_data) {
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

    output_var.putVar(startp, countp, data_to_save);
}

/** Saves a 2-dimensional data set with one variable for one timestep to file
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param v_out_data 2D vector of data to save
 */
void NcFileHandler::to_netcdf(std::string out_fpath, std::string variable_name, std::vector<std::vector<float>>& v_out_data) {
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
            data_to_save[lat][lon] = v_out_data[lat][lon];

    output_var.putVar(startp, countp, data_to_save);
}

/** Saves a 3-dimensional data set to file (3 dimensions: time x lat x lon )
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param out_data 3d array of data to save
 */
void NcFileHandler::to_netcdf(std::string out_fpath, std::string variable_name, float*** out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    netCDF::NcDim
        out_time_dim = output_file.addDim(time_name, n_time),
        out_lat_dim = output_file.addDim(lat_name, n_lat),
        out_lon_dim = output_file.addDim(lon_name, n_lon);

    netCDF::NcVar
        out_time_var = output_file.addVar(time_name, netCDF::ncDouble, out_time_dim),
        out_lat_var = output_file.addVar(lat_name, netCDF::ncFloat, out_lat_dim),
        out_lon_var = output_file.addVar(lon_name, netCDF::ncFloat, out_lon_dim);

    // Set attributes
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

    std::vector<netCDF::NcDim> dim_vector;
    dim_vector.push_back(out_time_dim);
    dim_vector.push_back(out_lat_dim);
    dim_vector.push_back(out_lon_dim);

    netCDF::NcVar
        output_var = output_file.addVar(variable_name, netCDF::ncFloat, dim_vector);

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

    for (size_t time = 0; time < n_time; time++) {
        startp[0] = time;
        float tmp[n_lat][n_lon];
        for (unsigned lat = 0; lat < n_lat; lat++) {
            for (unsigned lon = 0; lon < n_lon; lon++)
                tmp[lat][lon] = out_data[time][lat][lon];
        }
        output_var.putVar(startp, countp, tmp);
    }
}

/** Saves a 3-dimensional data set to file (3 dimensions: time x lat x lon )
 *
 * @param out_fpath output file path
 * @param variable_name name of the output variable
 * @param v_out_data 3D vector of data to save
 */
void NcFileHandler::to_netcdf(std::string out_fpath, std::string variable_name, std::vector<std::vector<std::vector<float>>>& v_out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    netCDF::NcDim
        out_time_dim = output_file.addDim(time_name, n_time),
        out_lat_dim = output_file.addDim(lat_name, n_lat),
        out_lon_dim = output_file.addDim(lon_name, n_lon);

    netCDF::NcVar
        out_time_var = output_file.addVar(time_name, netCDF::ncDouble, out_time_dim),
        out_lat_var = output_file.addVar(lat_name, netCDF::ncFloat, out_lat_dim),
        out_lon_var = output_file.addVar(lon_name, netCDF::ncFloat, out_lon_dim);

    // Set attributes
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

    for (std::pair<std::string, netCDF::NcVarAtt> att : lat_var.getAtts()) {
        if (att.second.getType().getName() == "char") {
            char value[att.second.getAttLength()];
            att.second.getValues(value);
            out_lat_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
        }
    }

    for (std::pair<std::string, netCDF::NcVarAtt> att : lon_var.getAtts()) {
        if (att.second.getType().getName() == "char") {
            char value[att.second.getAttLength()];
            att.second.getValues(value);
            out_lon_var.putAtt(att.first, att.second.getType(), att.second.getAttLength(), value);
        }
    }

    std::vector<netCDF::NcDim> dim_vector;
    dim_vector.push_back(out_time_dim);
    dim_vector.push_back(out_lat_dim);
    dim_vector.push_back(out_lon_dim);

    netCDF::NcVar output_var = output_file.addVar(variable_name, netCDF::ncFloat, dim_vector);

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

    for (size_t time = 0; time < n_time; time++) {
        startp[0] = time;
        float tmp[n_lat][n_lon];
        for (unsigned lat = 0; lat < n_lat; lat++) {
            for (unsigned lon = 0; lon < n_lon; lon++)
                tmp[lat][lon] = v_out_data[time][lat][lon];
        }
        output_var.putVar(startp, countp, tmp);
    }
}

/** Saves a 2-dimensional data set containing multiple variables for only one timestep to file
 *
 * @param out_fpath output file path
 * @param variable_names names of the output variables
 * @param out_data vector of 2D arrays of data
 */
void NcFileHandler::to_netcdf(std::string out_fpath, std::vector<std::string> variable_names, std::vector<float**> out_data) {
    netCDF::NcFile output_file(out_fpath, netCDF::NcFile::replace);

    netCDF::NcDim
        out_lat_dim = output_file.addDim(lat_name, n_lat),
        out_lon_dim = output_file.addDim(lon_name, n_lon);

    netCDF::NcVar
        out_lat_var = output_file.addVar(lat_name, netCDF::ncFloat, out_lat_dim),
        out_lon_var = output_file.addVar(lon_name, netCDF::ncFloat, out_lon_dim);

    out_lat_var.putVar(lat_values);
    out_lon_var.putVar(lon_values);

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
}
