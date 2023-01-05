// -*- lsst-c++ -*-

/**
 * @file CMethods.hxx
 * @brief declaration of the CMethod class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/BiasAdjustCXX
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
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef __CMETHODS__
#define __CMETHODS__

#include <iostream>
#include <stdexcept>
#include <vector>

struct AdjustmentSettings {
    AdjustmentSettings() : max_scaling_factor(10),    // maximum scaling factor
                           n_quantiles(250),          // number of quantiles to respect
                           interval31_scaling(true),  // calculate the means based on long-term 31-day moving windows or on the whole data at once
                           kind("add"){};             // adjustment kind => additive or multiplicative

    AdjustmentSettings(
        double max_scaling_factor,
        unsigned n_quantiles,
        bool interval31_scaling,
        std::string kind) : max_scaling_factor(max_scaling_factor),
                            n_quantiles(n_quantiles),
                            interval31_scaling(interval31_scaling),
                            kind(kind){};
    double max_scaling_factor;
    unsigned n_quantiles;
    bool interval31_scaling;
    std::string kind;
};

class CMethods {
   public:
    CMethods();
    ~CMethods();

    static std::vector<std::string> scaling_method_names;
    static std::vector<std::string> distribution_method_names;

    static double get_adjusted_scaling_factor(double factor, double max_factor);
    static std::vector<std::vector<float>> get_long_term_dayofyear(std::vector<float>& v_in);

    static void Linear_Scaling(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, AdjustmentSettings& settings);
    static void Variance_Scaling(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, AdjustmentSettings& settings);
    static void Delta_Method(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, AdjustmentSettings& settings);

    static std::vector<double> get_xbins(std::vector<float>& a, std::vector<float>& b, unsigned n_quantiles, std::string kind);
    static void Quantile_Mapping(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, AdjustmentSettings& settings);
    static void Quantile_Delta_Mapping(std::vector<float>& v_output, std::vector<float>& v_reference, std::vector<float>& v_control, std::vector<float>& v_scenario, AdjustmentSettings& settings);

   private:
};

#endif
