// -*- lsst-c++ -*-

/**
 * @file CMethods.hxx
 * @brief declaration of the CMethod class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 */

#ifndef __CMETHODS__
#define __CMETHODS__

#include <iostream>
#include <stdexcept>
#include <vector>
typedef void (*CM_Func_ptr_scaling)(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind);
typedef void (*CM_Func_ptr_quantile)(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind, unsigned n_quantiles);

class CMethods {
   public:
    CMethods();
    ~CMethods();

    static CM_Func_ptr_scaling get_cmethod_scaling(std::string method_name);
    static CM_Func_ptr_quantile get_cmethod_quantile(std::string method_name);

    static std::vector<std::string> scaling_method_names;
    static std::vector<std::string> quantile_method_names;

    static void Linear_Scaling(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind);
    static void Variance_Scaling(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind);
    static void Delta_Method(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind);

    static std::vector<double> get_xbins(std::vector<float>& a, std::vector<float>& b, unsigned n_quantiles, std::string kind);
    static void Quantile_Mapping(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind, unsigned n_quantiles);
    static void Quantile_Delta_Mapping(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind, unsigned n_quantiles);

   private:
};

#endif
