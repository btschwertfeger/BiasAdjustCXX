// -*- lsst-c++ -*-

/**
 * @file NcFileHandler.hxx
 * @brief File to declare NcFileHandler Class
 * @author Benjamin Thomas Schwertfeger
 * @email: contact@b-schwertfeger.de
 * @link https://github.com/btschwertfeger/BiasAdjustCXX
 *
 *  * Copyright (C) 2023 Benjamin Thomas Schwertfeger
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
 */

#ifndef __NcFileHandler__
#define __NcFileHandler__

#include <netcdf>

class NcFileHandler {
   public:
    NcFileHandler();
    NcFileHandler(std::string filepath, std::string variable_name, unsigned n_dimensions);
    ~NcFileHandler();

    void get_lat_timeseries_for_lon(std::vector<std::vector<float>>& v_out_arr, unsigned lon);
    void get_timeseries(std::vector<float>& v_out_arr, unsigned lat, unsigned lon);
    void get_timeseries(std::vector<float>& v_out_arr);

    void to_netcdf(std::string out_fpath, std::string variable_name, float* out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, std::vector<float>& v_out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, float** out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, std::vector<std::vector<float>>& v_data_out);

    void to_netcdf(std::string out_fpath, std::string variable_name, float*** out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, std::vector<std::vector<std::vector<float>>>& v_out_data);
    void to_netcdf(std::string out_fpath, std::vector<std::string> variable_names, std::vector<float**> out_data);

    static std::string time_name;
    static std::string lat_name;
    static std::string lon_name;

    static std::string units;
    static std::string lat_unit;
    static std::string lon_unit;

    std::string filepath;
    std::string var_name;
    netCDF::NcDim time_dim;
    netCDF::NcDim lat_dim;
    netCDF::NcDim lon_dim;

    unsigned int n_lat = 0;
    unsigned int n_lon = 0;
    unsigned int n_time = 0;

    float* lat_values = nullptr;
    float* lon_values = nullptr;
    double* time_values = nullptr;

    netCDF::NcFile* dataFile = nullptr;
    netCDF::NcVar data;
    netCDF::NcVar lon_var;
    netCDF::NcVar lat_var;
    netCDF::NcVar time_var;

    bool handles_file;
    unsigned n_dimensions;

    void read_dataset(std::string filepath, std::string variable, unsigned n_dimensiions);
    void close_file();

   private:
};

#endif
/**
 * * ----- ----- E O F ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ |
 */
