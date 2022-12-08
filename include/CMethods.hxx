// -*- lsst-c++ -*-

/**
 * @file CMethods.hxx
 * @brief declaration of the CMethod class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 *
 *  * Copyright (C) 2022  Benjamin Thomas Schwertfeger
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

#ifndef __CMETHODS__
#define __CMETHODS__

#include <iostream>
#include <stdexcept>
#include <vector>

class CMethods {
   public:
    CMethods();
    ~CMethods();

    static std::vector<std::string> scaling_method_names;
    static std::vector<std::string> distribution_method_names;

    static double get_adjusted_scaling_factor(double factor, double max_factor);
    static std::vector<std::vector<float>> get_long_term_dayofyear(std::vector<float>& v_in);

    static void Linear_Scaling_add(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, bool interval31_scaling = false);
    static void Linear_Scaling_mult(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, double max_scaling_factor, bool interval31_scaling = false);
    static void Variance_Scaling_add(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, double max_scaling_factor, bool interval31_scaling = false);
    static void Delta_Method_add(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, bool interval31_scaling = false);
    static void Delta_Method_mult(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, double max_scaling_factor, bool interval31_scaling = false);

    static std::vector<double> get_xbins(std::vector<float>& a, std::vector<float>& b, unsigned n_quantiles, std::string kind);

    static void Quantile_Mapping(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind, unsigned n_quantiles);
    static void Quantile_Delta_Mapping(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, std::string kind, unsigned n_quantiles);

   private:
};

#endif
