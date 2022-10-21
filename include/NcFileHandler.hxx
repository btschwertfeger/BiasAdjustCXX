// -*- lsst-c++ -*-

/**
 * @file NcFileHandler.hxx
 * @brief File to declare NcFileHandler Class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 */

#ifndef __NcFileHandler__
#define __NcFileHandler__

#include <netcdf>

class NcFileHandler {
   public:
    NcFileHandler();
    NcFileHandler(std::string filepath, std::string variable_name);
    ~NcFileHandler();

    void fill_lon_timeseries_for_lat(std::vector<std::vector<float>>& v_out_arr, unsigned lat);
    void fill_timeseries_for_location(std::vector<float>& v_out_arr, unsigned lat, unsigned lon);
    void to_netcdf(std::string out_fpath, std::string variable_name, float* out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, float* out_data, unsigned n_time);
    void to_netcdf(std::string out_fpath, std::string variable_name, std::vector<float>& v_out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, float** out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, double** out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, float*** out_data);
    void to_netcdf(std::string out_fpath, std::string variable_name, std::vector<std::vector<std::vector<float>>>& v_out_data);
    void to_netcdf(std::string out_fpath, std::vector<std::string> variable_names, std::vector<float**> out_data);

    static std::string time_name;
    static std::string lat_name;
    static std::string lon_name;

    static std::string units;
    static std::string lat_unit;
    static std::string lon_unit;

    std::string in_filename;

    std::string var_name;
    netCDF::NcDim lat_dim;
    netCDF::NcDim lon_dim;
    netCDF::NcDim time_dim;

    unsigned int n_lat;
    unsigned int n_lon;
    unsigned int n_time;

    float* lat_values;
    float* lon_values;
    double* time_values;

    netCDF::NcFile* dataFile;
    netCDF::NcVar data;
    netCDF::NcVar lon_var;
    netCDF::NcVar lat_var;
    netCDF::NcVar time_var;

    bool handles_file;
};

#endif
/**
 * * ----- ----- E O F ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ |
 */