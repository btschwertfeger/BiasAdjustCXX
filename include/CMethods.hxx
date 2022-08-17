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

    static CM_Func_ptr_simple get_cmethod_simple(std::string method_name);
    static CM_Func_ptr_quantile get_cmethod_quantile(std::string method_name);

    static std::vector<std::string> simple_method_names;
    static std::vector<std::string> quantile_method_names;
    static std::vector<std::string> all_method_names;

    static void Linear_Scaling(float* output, float* reference, float* control, float* scenario, unsigned, std::string n_time);
    static void Variance_Scaling(float* output, float* reference, float* control, float* scenario, unsigned, std::string n_time);
    static void Delta_Method(float* output, float* reference, float* control, float* scenario, unsigned, std::string n_time);

    static std::vector<double> get_xbins(float* a, float* b, unsigned n_quantiles, unsigned length, std::string kind);
    static void Quantile_Mapping(float* output, float* reference, float* control, float* scenario, unsigned n_time, std::string kind, unsigned n_quantiles);
    static void Quantile_Delta_Mapping(float* output, float* reference, float* control, float* scenario, unsigned n_time, std::string kind, unsigned n_quantiles);
};

#endif
