// -*- lsst-c++ -*-

/**
 * @file CMethods.cxx
 * @brief class/collection of procedures to bias adjust time series climate data
 * @author Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 *
 * * Copyright (C) 2022  Benjamin Thomas Schwertfeger
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
 *
 * * Notes:
 *  - Methods does not have to match the description of the mentioned papers
 *      - some of them are derived and improved
 *      - the references are just the base of the methods
 */

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Includes
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include "CMethods.hxx"

#include <algorithm>
#include <iostream>
#include <vector>

#include "MathUtils.hxx"
#include "math.h"

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Definitions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

std::vector<std::string> CMethods::scaling_method_names = {"linear_scaling", "variance_scaling", "delta_method"};
std::vector<std::string> CMethods::distribution_method_names = {"quantile_mapping", "quantile_delta_mapping"};

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Object Management
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

CMethods::CMethods() {}
CMethods::~CMethods() {}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Helper Functions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * returns 2d vector (dimensionss: [365, 31 * years]) containing the values with index -15...0...+15 around a day over all years
 * @param v_in input vector containing 365 entries for n years
 *
 */
std::vector<std::vector<float>> CMethods::get_long_term_dayofyear(std::vector<float> &v_in) {
    std::vector<std::vector<float>> v_out(365, std::vector<float>());
    if (v_in.size() % 365 != 0) throw std::runtime_error("The size of the input data does not match size % 365!");

    const unsigned n_years = (unsigned)(v_in.size() / 365);
    for (unsigned day = 0; day < 365; day++) {
        for (unsigned year = 0; year < n_years - 0; year++) {
            if (year == 0 && day < 15) {
                v_out[day].reserve(day + 15);
                v_out[day].insert(v_out[day].end(), v_in.begin(), v_in.begin() + day + 15);
            } else if (year == n_years - 1) {
                const int x = (year * 365 + day + 15);
                const int
                    end = v_in.size() > x ? x : v_in.size(),
                    start = day + year * 365 - 15;
                v_out[day].reserve(end - start);
                v_out[day].insert(v_out[day].end(), v_in.begin() + start, v_in.begin() + end);
            } else {
                v_out[day].reserve(30);
                v_out[day].insert(v_out[day].end(), v_in.begin() + day + year * 365 - 15, v_in.begin() + day + year * 365 + 15);
            }
        }
    }
    return v_out;
}

/**
 * returns either the factor or the maximum factor
 *
 * @param factor value to check
 * @param max_factor max allowed factor
 */
double CMethods::get_adjusted_scaling_factor(double factor, double max_factor) {
    return (factor > 0 && factor > max_factor)
               ? max_factor
           : (factor < 0 && factor < -max_factor)
               ? -max_factor
               : factor;
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Adjustment Methods
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * Method to adjust 1 dimensional climate data by the linear scaling method.
 * Based on the equations of Teutschbein, Claudia and Seibert, Jan (2012)
 * Bias correction of regional climate model simulations for hydrological climate-change impact studies:
 * Review and evaluation of different methods https://doi.org/10.1016/j.jhydrol.2012.05.052
 *
 * @param v_output output vector where to save the adjusted data
 * @param v_reference observation data (control period)
 * @param v_control modeled data of the control period
 * @param v_scenario data to adjust
 * @param max_scaling_factor maximum scaling for the multiplicative variant
 * @param interval_scaling365 calculate the means based on 30days around the adjusted time step instead on monthly means
 *
 * Add ('+'):
 *  (1.)    $T^{*}_{contr}(d)=T_{contr}(d)+\mu_{m}(T_{obs}(d))-\mu_{m}(T_{contr}(d))$
 *  (2.)    $T^{*}_{scen}(d)=T_{scen}(d)+\mu_{m}(T_{obs}(d))-\mu_{m}(T_{scen}(d))$
 * Mult ('*'):
 *  (3.)    $ T_{contr}^{*}(d) = \mu_{m}(T_{obs}(d))\cdot\left[\frac{T_{contr}(d)}{\mu_{m}(T_{contr}(d))}\right]$
 *  (4.)    $ T_{scen}^{*}(d) = \mu_{m}(T_{obs}(d))\cdot\left[\frac{T_{contr}(d)}{\mu_{m}(T_{scen}(d))}\right]$
 *
 */
void CMethods::Linear_Scaling_add(
    std::vector<float> &v_output,
    std::vector<float> &v_reference,
    std::vector<float> &v_control,
    std::vector<float> &v_scenario,
    bool interval_scaling365) {
    if (interval_scaling365) {
        if (!(v_reference.size() % 365 == 0 && v_control.size() % 365 == 0 && v_scenario.size() % 365 == 0))
            throw std::runtime_error("The time dimensions must have 365 entries for every year. Every year must be complete.");
        else {
            std::vector<float>
                ref_365_means,
                contr_365_means;
            std::vector<std::vector<float>>
                ref_long_term_dayofyear = get_long_term_dayofyear(v_reference),
                contr_long_term_dayofyear = get_long_term_dayofyear(v_control);

            for (unsigned day = 0; day < 365; day++) {
                ref_365_means.push_back(MathUtils::mean(ref_long_term_dayofyear[day]));
                contr_365_means.push_back(MathUtils::mean(contr_long_term_dayofyear[day]));
            }

            std::vector<float> scaling_factors;
            for (unsigned day = 0; day < 365; day++)
                scaling_factors.push_back(ref_365_means[day] - contr_365_means[day]);

            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_scenario[ts] + scaling_factors[ts % 365];
        }
    } else {
        const double scaling_factor = MathUtils::mean(v_reference) - MathUtils::mean(v_control);
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = v_scenario[ts] + scaling_factor;  // Eq. 1f.
    }
}
void CMethods::Linear_Scaling_mult(
    std::vector<float> &v_output,
    std::vector<float> &v_reference,
    std::vector<float> &v_control,
    std::vector<float> &v_scenario,
    double max_scaling_factor,
    bool interval_scaling365) {
    if (interval_scaling365) {
        if (!(v_reference.size() % 365 == 0 && v_control.size() % 365 == 0 && v_scenario.size() % 365 == 0))
            throw std::runtime_error("The time dimensions must have 365 entries for every year. Every year must be complete.");
        else {
            std::vector<float>
                ref_365_means,
                contr_365_means;
            std::vector<std::vector<float>> ref_long_term_dayofyear = get_long_term_dayofyear(v_reference);
            std::vector<std::vector<float>> contr_long_term_dayofyear = get_long_term_dayofyear(v_control);

            for (unsigned day = 0; day < 365; day++) {
                ref_365_means.push_back(MathUtils::mean(ref_long_term_dayofyear[day]));
                contr_365_means.push_back(MathUtils::mean(contr_long_term_dayofyear[day]));
            }

            std::vector<float> adj_scaling_factors;
            for (unsigned day = 0; day < 365; day++)
                adj_scaling_factors.push_back(get_adjusted_scaling_factor((ref_365_means[day] / contr_365_means[day]), max_scaling_factor));

            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_scenario[ts] * adj_scaling_factors[ts % 365];
        }
    } else {
        const double adjusted_scaling_factor = get_adjusted_scaling_factor(MathUtils::mean(v_reference) / MathUtils::mean(v_control), max_scaling_factor);
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = v_scenario[ts] * adjusted_scaling_factor;  // Eq. 3f.
    }
}

/**
 * Method to adjust 1 dimensional climate data by variance scaling method.
 * Based on the equations of Teutschbein, Claudia and Seibert, Jan (2012)
 * Bias correction of regional climate model simulations for hydrological climate-change impact studies:
 * Review and evaluation of different methods https://doi.org/10.1016/j.jhydrol.2012.05.052
 *
 * @param v_output output vector where to save the adjusted data
 * @param v_reference observation data (control period)
 * @param v_control modeled data of the control period
 * @param v_scenario data to adjust
 * @param interval_scaling365 is not an option here.
 *
 * (1.) $T^{*1}_{contr}(d)=T_{contr}(d)+\mu_{m}(T_{obs}(d))-\mu_{m}(T_{contr}(d))$
 * (2.) $T^{*1}_{scen}(d)=T_{scen}(d)+\mu_{m}(T_{obs}(d))-\mu_{m}(T_{scen}(d))$
 *
 * (3.) $T^{*2}_{contr}(d)=T^{*1}_{contr}(d)-\mu_{m}(T^{*1}_{contr}(d))$
 * (4.) $T^{*2}_{scen}(d)=T^{*1}_{scen}(d)-\mu_{m}(T^{*1}_{scen}(d))$
 *
 * (5.) $T^{*3}_{contr}(d)=T^{*2}_{scen}(d)\cdot\left[\frac{\sigma_{m}(T_{obs}(d))}{\sigma_{m}(T^{*2}_{contr}(d))}\right]$
 * (6.) $T^{*3}_{scen}(d)=T^{*2}_{scen}(d)\cdot\left[\frac{\sigma_{m}(T_{obs}(d))}{\sigma_{m}(T^{*2}_{contr}(d))}\right]$
 *
 * (7.) $T^{*}_{contr}(d)=T^{*3}_{contr}(d)+\mu_{m}(T^{*1}_{contr}(d))$
 * (8.) $T^{*}_{scen}(d)=T^{*3}_{scen}(d)+\mu_{m}(T^{*1}_{scen}(d))$
 */
void CMethods::Variance_Scaling_add(
    std::vector<float> &v_output,
    std::vector<float> &v_reference,
    std::vector<float> &v_control,
    std::vector<float> &v_scenario,
    double max_scaling_factor,
    bool interval_scaling365) {
    std::vector<float> LS_contr(v_reference.size());
    std::vector<float> LS_scen(v_reference.size());

    Linear_Scaling_add(LS_contr, v_reference, v_control, v_control, interval_scaling365);  // Eq. 1
    Linear_Scaling_add(LS_scen, v_reference, v_control, v_scenario, interval_scaling365);  // Eq. 2

    if (interval_scaling365) {
        std::vector<std::vector<float>>
            LS_contr_long_term_dayofyear = get_long_term_dayofyear(LS_contr),
            LS_scen_long_term_dayofyear = get_long_term_dayofyear(LS_scen);

        std::vector<float> LS_contr_365_means, LS_scen_365_means;
        for (unsigned day = 0; day < 365; day++) {
            LS_contr_365_means.push_back(MathUtils::mean(LS_contr_long_term_dayofyear[day]));
            LS_scen_365_means.push_back(MathUtils::mean(LS_scen_long_term_dayofyear[day]));
        }

        std::vector<float>
            VS1_contr(v_control.size()),
            VS1_scen(v_scenario.size());

        for (unsigned ts = 0; ts < v_control.size(); ts++)
            VS1_contr[ts] = LS_contr[ts] - LS_contr_365_means[ts % 365];  // Eq. 3

        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            VS1_scen[ts] = LS_scen[ts] - LS_scen_365_means[ts % 365];  // Eq. 4

        std::vector<std::vector<float>>
            ref_long_term_dayofyear = get_long_term_dayofyear(v_reference),
            VS1_contr_long_term_dayofyear = get_long_term_dayofyear(VS1_contr);

        std::vector<float> ref_365_standard_deviations, VS1_contr_365_standard_deviations;
        for (unsigned day = 0; day < 365; day++) {
            ref_365_standard_deviations.push_back(MathUtils::sd(ref_long_term_dayofyear[day]));
            VS1_contr_365_standard_deviations.push_back(MathUtils::sd(VS1_contr_long_term_dayofyear[day]));
        }

        std::vector<double> adj_scaling_factors;
        for (unsigned day = 0; day < 365; day++)
            adj_scaling_factors.push_back(get_adjusted_scaling_factor(ref_365_standard_deviations[day] / VS1_contr_365_standard_deviations[day], max_scaling_factor));

        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (VS1_scen[ts] * adj_scaling_factors[ts % 365]) + LS_scen_365_means[ts % 365];  // Eq. 6 and 8

    } else {
        double
            LS_contr_mean = MathUtils::mean(LS_contr),
            LS_scen_mean = MathUtils::mean(LS_scen);

        std::vector<float>
            VS1_contr(v_control.size()),
            VS1_scen(v_scenario.size());

        for (unsigned ts = 0; ts < v_control.size(); ts++)
            VS1_contr[ts] = LS_contr[ts] - LS_contr_mean;  // Eq. 3

        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            VS1_scen[ts] = LS_scen[ts] - LS_scen_mean;  // Eq. 4

        const double adjusted_scaling_factor = get_adjusted_scaling_factor(MathUtils::sd(v_reference) / MathUtils::sd(VS1_contr), max_scaling_factor);
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (VS1_scen[ts] * adjusted_scaling_factor) + LS_scen_mean;  // Eq. 6 and 8
    }
}

/**
 * Method to adjust 1 dimensional climate data by delta method.
 * Based on Beyer, R. and Krapp, M. and Manica, A.: An empirical evaluation of bias correction methods for palaeoclimate simulations (https://doi.org/10.5194/cp-16-1493-2020)
 *
 * NOTE: v_reference.size() must be equal to v_scenario.size()
 * @param v_output output vector where to save the adjusted data
 * @param v_reference observation data (control period)
 * @param v_control modeled data of the control period
 * @param v_scenario data to adjust
 * @param max_scaling_factor maximum scaling for the multiplicative variant
 * @param interval_scaling365 calculate the means based on 30days around the adjusted time step instead on monthly means
 *
 * Add (+):
 *   (1.) $T^{*}_{contr}(d) = T_{contr}(d) + (\mu_{m}(T_{scen}(d)) - \mu_{m}(T_{obs}(d)))$
 * Mult (*):
 *   (2.) $T^{*}_{contr}(d) = T_{contr}(d) \cdot \left[\frac{\mu_{m}(T_{scen}(d))}{\mu_{m}(T_{obs}(d))}\right]$
 *
 */
void CMethods::Delta_Method_add(
    std::vector<float> &v_output,
    std::vector<float> &v_reference,
    std::vector<float> &v_control,
    std::vector<float> &v_scenario,
    bool interval_scaling365) {
    if (interval_scaling365) {
        if (!(v_reference.size() % 365 == 0 && v_control.size() % 365 == 0 && v_scenario.size() % 365 == 0))
            throw std::runtime_error("The time dimensions must have 365 entries for every year. Every year must be complete.");
        else {
            std::vector<float>
                contr_365_means,
                scen_365_means;
            std::vector<std::vector<float>>
                contr_long_term_dayofyear = get_long_term_dayofyear(v_control),
                scen_long_term_dayofyear = get_long_term_dayofyear(v_scenario);

            for (unsigned day = 0; day < 365; day++) {
                contr_365_means.push_back(MathUtils::mean(contr_long_term_dayofyear[day]));
                scen_365_means.push_back(MathUtils::mean(scen_long_term_dayofyear[day]));
            }

            for (unsigned ts = 0; ts < v_reference.size(); ts++)
                v_output[ts] = v_reference[ts] + (scen_365_means[ts % 365] - contr_365_means[ts % 365]);
        }
    } else {
        const double scaling_factor = MathUtils::mean(v_scenario) - MathUtils::mean(v_control);
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = v_reference[ts] + scaling_factor;  // Eq. 1
    }
}
void CMethods::Delta_Method_mult(
    std::vector<float> &v_output,
    std::vector<float> &v_reference,
    std::vector<float> &v_control,
    std::vector<float> &v_scenario,
    double max_scaling_factor,
    bool interval_scaling365) {
    if (interval_scaling365) {
        if (!(v_reference.size() % 365 == 0 && v_control.size() % 365 == 0 && v_scenario.size() % 365 == 0))
            throw std::runtime_error("The time dimensions must have 365 entries for every year. Every year must be complete.");
        else {
            std::vector<float>
                contr_365_means,
                scen_365_means;
            std::vector<std::vector<float>>
                contr_long_term_dayofyear = get_long_term_dayofyear(v_control),
                scen_long_term_dayofyear = get_long_term_dayofyear(v_scenario);

            for (unsigned day = 0; day < 365; day++) {
                contr_365_means.push_back(MathUtils::mean(contr_long_term_dayofyear[day]));
                scen_365_means.push_back(MathUtils::mean(scen_long_term_dayofyear[day]));
            }

            std::vector<float> adj_scaling_factors;
            for (unsigned day = 0; day < 365; day++)
                adj_scaling_factors.push_back(get_adjusted_scaling_factor(scen_365_means[day] / contr_365_means[day], max_scaling_factor));

            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_reference[ts] * adj_scaling_factors[ts % 365];
        }
    } else {
        const double adjusted_scaling_factor = get_adjusted_scaling_factor(MathUtils::mean(v_scenario) / MathUtils::mean(v_control), max_scaling_factor);
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = v_reference[ts] * adjusted_scaling_factor;  // Eq. 2
    }
}

/**
 *
 * @param a time series
 * @param b time series
 * @param kind regular or bounded: defines if min is the real minimum or set to 0
 */
std::vector<double> CMethods::get_xbins(std::vector<float> &a, std::vector<float> &b, unsigned n_quantiles, std::string kind) {
    if (kind == "regular") {
        const double
            a_max = *std::max_element(std::begin(a), std::end(a)),
            a_min = *std::min_element(std::begin(a), std::end(a)),
            b_max = *std::max_element(std::begin(b), std::end(b)),
            b_min = *std::min_element(std::begin(b), std::end(b));

        const double
            global_max = std::max(a_max, b_max),
            global_min = std::min(a_min, b_min);
        const double wide = std::abs(global_max - global_min) / n_quantiles;

        std::vector<double> v_xbins(0);
        v_xbins.push_back((double)global_min);
        while (v_xbins[v_xbins.size() - 1] < global_max)
            v_xbins.push_back(v_xbins[v_xbins.size() - 1] + wide);
        return v_xbins;

    } else if (kind == "bounded") {
        const double
            a_max = *std::max_element(std::begin(a), std::end(a)),
            b_max = *std::max_element(std::begin(b), std::end(b));
        const double global_max = std::max(a_max, b_max);
        const double wide = global_max / n_quantiles;

        std::vector<double> v_xbins(0);
        v_xbins.push_back((double)0);
        while (v_xbins[v_xbins.size() - 1] < global_max)
            v_xbins.push_back(v_xbins[v_xbins.size() - 1] + wide);
        return v_xbins;
    } else {
        throw std::runtime_error("Unknown kind " + kind + " for get_xbins-function.");
        return std::vector<double>(0);  // just to avoid warnings ...
    }
}

/**
 * Quantile Mapping bias correction based on
 * Tong, Y., Gao, X., Han, Z. et al. Bias correction of temperature and precipitation over China for RCM
 * simulations using the QM and QDM methods. Clim Dyn 57, 1425–1443 (2021). https://doi.org/10.1007/s00382-020-05447-4
 *
 * @param v_output output array where to save the adjusted data
 * @param v_reference observation data (control period)
 * @param v_control modeled data of the control period
 * @param v_scenario data to adjust
 * @param kind type of adjustment; additive or multiplicative
 * @param n_quantiles number of quantiles to use
 *
 * (add):
 *      (1.) $T^{*QM}_{sim,p}(d)=F^{-1}_{obs,h} \left\{F_{sim,h}\left[T_{sim,p}(d)\right]\right\}$
 * (mult):
 *      same but experimental
 */
void CMethods::Quantile_Mapping(
    std::vector<float> &v_output,
    std::vector<float> &v_reference,
    std::vector<float> &v_control,
    std::vector<float> &v_scenario,
    std::string kind,
    unsigned n_quantiles) {
    if (kind == "add" || kind == "+") {
        std::vector<double> v_xbins = get_xbins(v_reference, v_control, n_quantiles, "regular");

        std::vector<int>  // ? create CDFs
            vi_ref_cdf = MathUtils::get_cdf(v_reference, v_xbins),
            vi_contr_cdf = MathUtils::get_cdf(v_control, v_xbins);

        std::vector<double>  // ? change CDF values to type double
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end());

        // ? Interpolate
        std::vector<double> cdf_values;
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            cdf_values.push_back(MathUtils::interpolate(v_xbins, contr_cdf, (double)v_scenario[ts], false));

        // ? Invert in inversed CDF and return
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (float)MathUtils::interpolate(ref_cdf, v_xbins, cdf_values[ts], false);

    } else if (kind == "mult" || kind == "*") {
        std::vector<double> v_xbins = get_xbins(v_reference, v_control, n_quantiles, "bounded");

        std::vector<int>  // ? create CDF
            vi_ref_cdf = MathUtils::get_cdf(v_reference, v_xbins),
            vi_contr_cdf = MathUtils::get_cdf(v_control, v_xbins);

        std::vector<double>  // ? change to double
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end());

        // ? Interpolate
        std::vector<double> cdf_values;
        for (unsigned ts = 0; ts < v_scenario.size(); ts++) {
            double y = MathUtils::interpolate(v_xbins, contr_cdf, (double)v_scenario[ts], true);
            cdf_values.push_back((y >= 0) ? y : 0);
        }

        // ? Invert in inversed CDF and return
        for (unsigned ts = 0; ts < v_scenario.size(); ts++) {
            float y = (float)MathUtils::interpolate(ref_cdf, v_xbins, cdf_values[ts], true);
            v_output[ts] = (y >= 0) ? y : 0;
        }
    } else
        throw std::runtime_error("Adjustment kind " + kind + " unknown for quantile mapping!");
}

/**
 * Quantile Delta Mapping bias correction based on
 * Tong, Y., Gao, X., Han, Z. et al. Bias correction of temperature and precipitation over China for RCM
 * simulations using the QM and QDM methods. Clim Dyn 57, 1425–1443 (2021). https://doi.org/10.1007/s00382-020-05447-4
 *
 * @param v_output output vector where to save the adjusted data
 * @param v_reference observation data (control period)
 * @param v_control modeled data of the control period
 * @param v_scenario data to adjust
 * @param kind type of adjustment; additive or multiplicative (absolute or relative change)
 * @param n_quantiles number of quantiles to use
 *
 * Add (+):
 *  (1.1) $\varepsilon(t) = F^{(t)}_{sim,p}\left[T_{sim,p}(t)\right]$
 *  (1.2) $\T^{*1QDM}_{sim,p}(t) = F^{-1}_{obs,h}\left[\varepsilon(t)\right]$
 *  (1.3) $\Delta(t) = F^{-1}_{sim,p}(t)\left[\varepsilon(t)\right] = T_{sim,p}(t)-F^{-1}_{sim,h} \left\{F^{}_{sim,p}(t)\left[T_{sim,p}(t)\right]\right\}$
 *  (1.4) $T^{*QDM}_{sim,p}(t) = T^{*1QDM}_{sim,p}(t) + \Delta(t)$
 *
 * Mult (*):
 *   (1.1) --//--
 *   (1.2) --//--
 *   (2.3) \Delta(i) & = \frac{ F^{-1}_{sim,p}\left[\varepsilon(i)\right] }{ F^{-1}_{sim,h}\left[\varepsilon(i)\right] } \\
 *                   & = \frac{ X_{sim,p}(i) }{ F^{-1}_{sim,h}\left\{F^{}_{sim,p}\left[X_{sim,p}(i)\right]\right\} }
 *   (2.4) X^{*QDM}_{sim,p}(i) = X^{QDM(1)}_{sim,p}(i) \cdot \Delta(i)
 */
void CMethods::Quantile_Delta_Mapping(
    std::vector<float> &v_output,
    std::vector<float> &v_reference,
    std::vector<float> &v_control,
    std::vector<float> &v_scenario,
    std::string kind,
    unsigned n_quantiles) {
    if (kind == "add" || kind == "+") {
        std::vector<double> v_xbins = get_xbins(v_reference, v_control, n_quantiles, "regular");

        std::vector<int>  // ? create CDF
            vi_ref_cdf = MathUtils::get_cdf(v_reference, v_xbins),
            vi_contr_cdf = MathUtils::get_cdf(v_control, v_xbins),
            vi_scen_cdf = MathUtils::get_cdf(v_scenario, v_xbins);

        std::vector<double>  // ? change to double
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end()),
            scen_cdf(vi_scen_cdf.begin(), vi_scen_cdf.end());

        std::vector<double> epsilon;
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            epsilon.push_back(MathUtils::interpolate(v_xbins, scen_cdf, v_scenario[ts], false));

        std::vector<double> QDM1;  // insert simulated values into inverse cdf of observed
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            QDM1.push_back(MathUtils::interpolate(ref_cdf, v_xbins, epsilon[ts], false));

        // ? Invert, insert in inversed CDF and return
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (float)(QDM1[ts] + v_scenario[ts] - MathUtils::interpolate(contr_cdf, v_xbins, epsilon[ts], false));  // Eq. 2f.

    } else if (kind == "mult" || kind == "*") {
        std::vector<double> v_xbins = get_xbins(v_reference, v_control, n_quantiles, "bounded");

        // ? create CDF
        std::vector<int>
            vi_ref_cdf = MathUtils::get_cdf(v_reference, v_xbins),
            vi_contr_cdf = MathUtils::get_cdf(v_control, v_xbins),
            vi_scen_cdf = MathUtils::get_cdf(v_scenario, v_xbins);

        std::vector<double>
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end()),
            scen_cdf(vi_scen_cdf.begin(), vi_scen_cdf.end());

        std::vector<double> epsilon;
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            epsilon.push_back(MathUtils::interpolate(v_xbins, scen_cdf, v_scenario[ts], false));

        std::vector<double> QDM1;  // insert simulated values into inverse cdf of observed
        for (unsigned ts = 0; ts < v_scenario.size(); ts++) QDM1.push_back(MathUtils::interpolate(ref_cdf, v_xbins, epsilon[ts], false));

        // ? Invert, insert in inversed CDF and return
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (float)(QDM1[ts] * (v_scenario[ts] / MathUtils::interpolate(contr_cdf, v_xbins, epsilon[ts], false)));  // Eq. 2.3f.
    } else
        throw std::runtime_error("Adjustment kind " + kind + " unknown for quantile delta mapping!");
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        E O F
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */
