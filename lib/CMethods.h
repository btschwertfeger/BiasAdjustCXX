// -*- lsst-c++ -*-

/**
 * @file CMethods.h
 * @brief declaration of the CMethod class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 */

#ifndef __CMETHODS__
#define __CMETHODS__

#include <iostream>

typedef void (*CM_Func_ptr_simple)(float* output, float* ref, float* contr, float* scen, unsigned n_time, std::string kind);
typedef void (*CM_Func_ptr_quantile)(float* output, float* ref, float* contr, float* scen, unsigned n_time, std::string kind, unsigned n_quantiles);

class CMethods {
   public:
    CMethods();
    ~CMethods();

    static CM_Func_ptr_simple get_cmethod_simple(std::string);
    static CM_Func_ptr_quantile get_cmethod_quantile(std::string);

    static std::vector<std::string> simple_method_names;
    static std::vector<std::string> quantile_method_names;
    static std::vector<std::string> all_method_names;

    static void Linear_Scaling(float*, float*, float*, float*, unsigned, std::string);
    static void Variance_Scaling(float*, float*, float*, float*, unsigned, std::string);
    static void Delta_Change(float*, float*, float*, float*, unsigned, std::string);

    static std::vector<double> get_xbins(float*, float*, unsigned, unsigned);
    static void Quantile_Mapping(float*, float*, float*, float*, unsigned, std::string, unsigned);
    static void Quantile_Delta_Mapping(float*, float*, float*, float*, unsigned, std::string, unsigned);
};

#endif