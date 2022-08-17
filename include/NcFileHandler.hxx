// -*- lsst-c++ -*-

/**
 * @file NcFileHandler.h
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
    NcFileHandler(std::string, std::string);
    ~NcFileHandler();

    void load_whole_dataset(float***, std::string);
    void fill_lon_timeseries_for_lat(float**, unsigned);
    void fill_timeseries_for_location(float*, unsigned, unsigned);
    void save_to_netcdf(std::string, std::string, float**);
    void save_to_netcdf(std::string, std::string, double**);
    void save_to_netcdf(std::string, std::string, float***);
    void save_to_netcdf(std::string, std::vector<std::string>, std::vector<float**>);

    void test();

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
};

#endif
/**
 * * ----- ----- E O F ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ ----- ------ |
 */