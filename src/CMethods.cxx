// -*- lsst-c++ -*-

/**
 * @file CMethods.cxx
 * @brief Class/collection of procedures to bias adjust time series climate data
 * @author Benjamin Thomas Schwertfeger
 * @email: contact@b-schwertfeger.de
 * @link https://github.com/btschwertfeger/BiasAdjustCXX
 *
 * * Copyright (C) 2023 Benjamin Thomas Schwertfeger
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
 * Get 365 long-term 31-day moving windows
 * When using long-term 31-day intervals, leap years should not be included and every year must be complete.
 *
 * @param v_in input vector containing 365 entries for $n$ years
 * @return 2D vector (dimensions: [365, 31 * y]) containing the values with index -15...0...+15 around a day over all years
 */
std::vector<std::vector<float>> CMethods::get_long_term_dayofyear(std::vector<float>& v_in) {
    std::vector<std::vector<float>> v_out(365, std::vector<float>());
    if (v_in.size() % 365 != 0)
        throw std::runtime_error(
            "The size of the time dimension of the "
            "input data does not match `size` % 365!"
        );

    const unsigned n_years = (unsigned)(v_in.size() / 365);
    for (unsigned day = 0; day < 365; day++) {
        for (unsigned year = 0; year < n_years - 0; year++) {
            if (year == 0 && day < 16) {
                v_out[day].reserve(day + 16);
                v_out[day].insert(
                    v_out[day].end(),
                    v_in.begin(),
                    v_in.begin() + day + 16
                );

            } else if (year == n_years - 1) {
                const int x = (year * 365 + day + 16);
                const int
                    end = v_in.size() > x ? x : v_in.size(),
                    start = day + year * 365 - 15;
                v_out[day].reserve(end - start);
                v_out[day].insert(
                    v_out[day].end(),
                    v_in.begin() + start,
                    v_in.begin() + end
                );

            } else {
                v_out[day].reserve(31);
                v_out[day].insert(
                    v_out[day].end(),
                    v_in.begin() + day + year * 365 - 15,
                    v_in.begin() + day + year * 365 + 16
                );
            }
        }
    }
    return v_out;
}

/**
 * Checks and returns the desired scaling factor based on `max_factor`
 *
 * @param factor value to check
 * @param max_factor max allowed factor (will be casted to positive value, cannot be zero)
 * @return either `factor` if ths ok or `max_factor`
 */
double CMethods::get_adjusted_scaling_factor(double factor, double max_factor) {
    const double abs_max_factor = std::abs(max_factor);
    return (factor > 0 && factor > abs_max_factor)
               ? abs_max_factor
           : (factor < 0 && factor < -abs_max_factor)
               ? -abs_max_factor
               : factor;
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Adjustment Methods
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * Method to adjust 1-dimensional climate data by the linear scaling method.
 * Based on the equations of Teutschbein, Claudia and Seibert, Jan (2012)
 * Bias correction of regional climate model simulations for hydrological climate-change impact studies:
 * Review and evaluation of different methods (https://doi.org/10.1016/j.jhydrol.2012.05.052)
 *
 * @param v_output 1D output vector that stores the adjusted time series
 * @param v_reference 1D reference time series (control period)
 * @param v_control 1D modeled time series (control period)
 * @param v_scenario 1D time series to adjust (scenario period)
 * @param settings struct with attributes:
 *      double max_scaling_factor,
 *      unsigned n_quantiles,
 *      bool interval31_scaling,
 *      std::string kind
 *
 * Add ('+'):
 *  (1.)    $T^{*}_{contr}(d) = T_{contr}(d) + \mu_{m}(T_{obs}(d)) - \mu_{m}(T_{contr}(d))$
 *  (2.)    $T^{*}_{scen}(d) = T_{scen}(d) + \mu_{m}(T_{obs}(d)) - \mu_{m}(T_{scen}(d))$
 * Mult ('*'):
 *  (3.)    $ T_{contr}^{*}(d) = \mu_{m}(T_{obs}(d)) \cdot \left[\frac{T_{contr}(d)}{\mu_{m}(T_{contr}(d))}\right]$
 *  (4.)    $ T_{scen}^{*}(d) = \mu_{m}(T_{obs}(d)) \cdot \left[\frac{T_{contr}(d)}{\mu_{m}(T_{scen}(d))}\right]$
 */

void CMethods::Linear_Scaling(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    AdjustmentSettings& settings
) {
    if (!settings.interval31_scaling) {
        if (settings.kind == "add" || settings.kind == "+") {
            const double scaling_factor = MathUtils::mean(v_reference) - MathUtils::mean(v_control);
            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_scenario[ts] + scaling_factor;  // Eq. 2

        } else if (settings.kind == "mult" || settings.kind == "*") {
            const double
                adjusted_scaling_factor = MathUtils::ensure_devidable(
                    MathUtils::mean(v_reference), MathUtils::mean(v_control),
                    settings.max_scaling_factor
                );
            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_scenario[ts] * adjusted_scaling_factor;  // Eq. 4

        } else
            throw std::runtime_error("Unknown Adjustment kind" + settings.kind + " for Linear Scaling.");

    } else {
        std::vector<float>
            ref_365_means,
            contr_365_means;

        std::vector<std::vector<float>>
            ref_long_term_dayofyear = get_long_term_dayofyear(v_reference),
            contr_long_term_dayofyear = get_long_term_dayofyear(v_control);

        // ? compute 365 means based on long-term 31-day moving windows
        for (unsigned day = 0; day < 365; day++) {
            ref_365_means.push_back(MathUtils::mean(ref_long_term_dayofyear[day]));
            contr_365_means.push_back(MathUtils::mean(contr_long_term_dayofyear[day]));
        }

        if (settings.kind == "add" || settings.kind == "+") {
            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_scenario[ts] + (ref_365_means[ts % 365] - contr_365_means[ts % 365]);  // Eq. 2

        } else if (settings.kind == "mult" || settings.kind == "*") {
            std::vector<float> adj_scaling_factors;
            for (unsigned day = 0; day < 365; day++)
                adj_scaling_factors.push_back(
                    MathUtils::ensure_devidable(
                        ref_365_means[day], contr_365_means[day],
                        settings.max_scaling_factor
                    )
                );

            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_scenario[ts] * adj_scaling_factors[ts % 365];  // Eq. 4

        } else
            throw std::runtime_error("Unknown Adjustment kind" + settings.kind + " for Linear Scaling.");
    }
}

/**
 * Method to adjust 1 dimensional climate data by variance scaling method.
 * Based on the equations of Teutschbein, Claudia and Seibert, Jan (2012)
 * Bias correction of regional climate model simulations for hydrological climate-change impact studies:
 * Review and evaluation of different methods (https://doi.org/10.1016/j.jhydrol.2012.05.052)
 *
 * @param v_output 1D output vector that stores the adjusted time series
 * @param v_reference 1D reference time series (control period)
 * @param v_control 1D modeled time series (control period)
 * @param v_scenario 1D time series to adjust (scenario period)
 * @param settings struct with attributes:
 *      double max_scaling_factor = 10,
 *      unsigned n_quantiles = 250,
 *      bool interval31_scaling = false,
 *      std::string kind
 *
 * (1.) $T^{*1}_{contr}(d) = T_{contr}(d) + \mu_{m}(T_{obs}(d)) - \mu_{m}(T_{contr}(d))$
 * (2.) $T^{*1}_{scen}(d) = T_{scen}(d) + \mu_{m}(T_{obs}(d)) - \mu_{m}(T_{scen}(d))$
 *
 * (3.) $T^{*2}_{contr}(d) = T^{*1}_{contr}(d) - \mu_{m}(T^{*1}_{contr}(d))$
 * (4.) $T^{*2}_{scen}(d) = T^{*1}_{scen}(d) - \mu_{m}(T^{*1}_{scen}(d))$
 *
 * (5.) $T^{*3}_{contr}(d) = T^{*2}_{scen}(d) \cdot \left[\frac{\sigma_{m}(T_{obs}(d))}{\sigma_{m}(T^{*2}_{contr}(d))}\right]$
 * (6.) $T^{*3}_{scen}(d) = T^{*2}_{scen}(d) \cdot \left[\frac{\sigma_{m}(T_{obs}(d))}{\sigma_{m}(T^{*2}_{contr}(d))}\right]$
 *
 * (7.) $T^{*}_{contr}(d) = T^{*3}_{contr}(d) + \mu_{m}(T^{*1}_{contr}(d))$
 * (8.) $T^{*}_{scen}(d) = T^{*3}_{scen}(d) + \mu_{m}(T^{*1}_{scen}(d))$
 */
void CMethods::Variance_Scaling(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    AdjustmentSettings& settings
) {
    std::vector<float> LS_contr(v_reference.size());
    std::vector<float> LS_scen(v_reference.size());

    Linear_Scaling(LS_contr, v_reference, v_control, v_control, settings);  // Eq. 1
    Linear_Scaling(LS_scen, v_reference, v_control, v_scenario, settings);  // Eq. 2

    if (!settings.interval31_scaling) {
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

        const double
            adjusted_scaling_factor = MathUtils::ensure_devidable(
                MathUtils::sd(v_reference), MathUtils::sd(VS1_contr),
                settings.max_scaling_factor
            );

        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (VS1_scen[ts] * adjusted_scaling_factor) + LS_scen_mean;  // Eq. 6 and 8

    } else {
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

        // ? compute 365 standard deviations based on long-term 31-day moving windows
        std::vector<float> ref_365_standard_deviations, VS1_contr_365_standard_deviations;
        for (unsigned day = 0; day < 365; day++) {
            ref_365_standard_deviations.push_back(MathUtils::sd(ref_long_term_dayofyear[day]));
            VS1_contr_365_standard_deviations.push_back(MathUtils::sd(VS1_contr_long_term_dayofyear[day]));
        }

        std::vector<double> adj_scaling_factors;
        for (unsigned day = 0; day < 365; day++)
            adj_scaling_factors.push_back(MathUtils::ensure_devidable(
                ref_365_standard_deviations[day], VS1_contr_365_standard_deviations[day],
                settings.max_scaling_factor
            ));

        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (VS1_scen[ts] * adj_scaling_factors[ts % 365]) + LS_scen_365_means[ts % 365];  // Eq. 6 and Eq. 8
    }
}

/**
 * Method to adjust 1-dimensional climate data by the delta method.
 * Based on Beyer, R. and Krapp, M. and Manica, A.: An empirical evaluation of bias
 * correction methods for palaeoclimate simulations (https://doi.org/10.5194/cp-16-1493-2020)
 *
 * NOTE: `v_reference.size()` must be equal to `v_scenario.size()`
 * @param v_output 1D output vector that stores the adjusted time series
 * @param v_reference 1D reference time series (control period)
 * @param v_control 1D modeled time series (control period)
 * @param v_scenario 1D time series to adjust (scenario period)
 * @param settings struct with attributes:
 *      double max_scaling_factor,
 *      unsigned n_quantiles,
 *      bool interval31_scaling,
 *      std::string kind
 *
 * Add (+):
 *   (1.) $T^{*}_{contr}(d) = T_{contr}(d) + (\mu_{m}(T_{scen}(d)) - \mu_{m}(T_{obs}(d)))$
 * Mult (*):
 *   (2.) $T^{*}_{contr}(d) = T_{contr}(d) \cdot \left[\frac{\mu_{m}(T_{scen}(d))}{\mu_{m}(T_{obs}(d))}\right]
 */
void CMethods::Delta_Method(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    AdjustmentSettings& settings
) {
    if (v_reference.size() != v_scenario.size())
        throw std::runtime_error(
            "Time dimension of reference and scenario input files does "
            "not have the same length! This is required for the delta method."
        );

    if (!settings.interval31_scaling) {
        if (settings.kind == "add" || settings.kind == "+") {
            const double scaling_factor = MathUtils::mean(v_scenario) - MathUtils::mean(v_control);
            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_reference[ts] + scaling_factor;  // Eq. 1

        } else if (settings.kind == "mult" || settings.kind == "*") {
            const double adjusted_scaling_factor = MathUtils::ensure_devidable(
                MathUtils::mean(v_scenario), MathUtils::mean(v_control),
                settings.max_scaling_factor
            );
            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_reference[ts] * adjusted_scaling_factor;  // Eq. 2

        } else
            throw std::runtime_error("Unknown Adjustment kind" + settings.kind + " for Delta Method.");

    } else {
        std::vector<float>
            contr_365_means,
            scen_365_means;

        std::vector<std::vector<float>>
            contr_long_term_dayofyear = get_long_term_dayofyear(v_control),
            scen_long_term_dayofyear = get_long_term_dayofyear(v_scenario);

        // ? compute 365 means based on long-term 31-day moving windows
        for (unsigned day = 0; day < 365; day++) {
            contr_365_means.push_back(MathUtils::mean(contr_long_term_dayofyear[day]));
            scen_365_means.push_back(MathUtils::mean(scen_long_term_dayofyear[day]));
        }

        if (settings.kind == "add" || settings.kind == "+") {
            for (unsigned ts = 0; ts < v_reference.size(); ts++)
                v_output[ts] = v_reference[ts] + (scen_365_means[ts % 365] - contr_365_means[ts % 365]);  // Eq. 1

        } else if (settings.kind == "mult" || settings.kind == "*") {
            std::vector<float> adj_scaling_factors;
            for (unsigned day = 0; day < 365; day++)
                adj_scaling_factors.push_back(
                    MathUtils::ensure_devidable(
                        scen_365_means[day], contr_365_means[day],
                        settings.max_scaling_factor
                    )
                );

            for (unsigned ts = 0; ts < v_scenario.size(); ts++)
                v_output[ts] = v_reference[ts] * adj_scaling_factors[ts % 365];  // Eq. 2

        } else
            throw std::runtime_error("Unknown Adjustment kind" + settings.kind + " for Delta Method.");
    }
}

/**
 * Returns the probability boundaries based on two input time series
 * -> This is required to compute the bin boundaries for the Probability Density and Cumulative Distribution Funtion.
 * -> Used by Quantile and Quantile Delta Mapping by invoking this funtion to get the bins based on the time
 *    series of the control period.
 *
 * @param a time series
 * @param b another time series
 * @param n_quantiles number of quantiles to use/respect
 * @param kind regular or bounded: defines if mininum value of a (climate) variable can be lower than in the data
 *             or is set to 0 (ratio based variables => 0)
 * @return the probability boundaries mentioned above
 */
std::vector<double> CMethods::get_xbins(
    std::vector<float>& a,
    std::vector<float>& b,
    unsigned n_quantiles,
    std::string kind
) {
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
 * @param v_output 1D output vector that stores the adjusted time series
 * @param v_reference 1D reference time series (control period)
 * @param v_control 1D modeled time series (control period)
 * @param v_scenario 1D time series to adjust (scenario period)
 * @param settings struct with attributes:
 *      double max_scaling_factor,
 *      unsigned n_quantiles,
 *      bool interval31_scaling,
 *      std::string kind
 *
 * (add):
 *      (1.) $T^{*QM}_{sim,p}(d) = F^{-1}_{obs,h} \left\{F_{sim,h}\left[T_{sim,p}(d)\right]\right\}$
 * (mult):
 *      same but with bounded values => 0 is the minimum value
 */
void CMethods::Quantile_Mapping(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    AdjustmentSettings& settings
) {
    const bool isAdd = (settings.kind == "add" || settings.kind == "+") ? true : false;
    if (!isAdd && !(settings.kind == "mult" || settings.kind == "*"))
        throw std::runtime_error("Adjustment kind " + settings.kind + " unknown for quantile mapping!");

    std::vector<double>
        v_xbins = get_xbins(
            v_reference,
            v_control,
            settings.n_quantiles,
            (isAdd) ? "regular" : "bounded"
        );

    std::vector<int>  // ? create CDFs
        vi_ref_cdf = MathUtils::get_cdf(v_reference, v_xbins),
        vi_contr_cdf = MathUtils::get_cdf(v_control, v_xbins);

    std::vector<double>  // ? change CDF values to type double
        ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
        contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end());

    std::vector<double> cdf_values;
    if (isAdd) {
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            cdf_values.push_back(
                MathUtils::interpolate(v_xbins, contr_cdf, (double)v_scenario[ts], false)
            );  // Eq. 1

        // ? Invert in inversed CDF and return
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (float)MathUtils::interpolate(ref_cdf, v_xbins, cdf_values[ts], false);  // Eq. 1

    } else {
        for (unsigned ts = 0; ts < v_scenario.size(); ts++) {
            double y = MathUtils::interpolate(v_xbins, contr_cdf, (double)v_scenario[ts], true);  // Eq. 1
            cdf_values.push_back((y >= 0) ? y : 0);
        }

        // ? Invert in inversed CDF and return
        for (unsigned ts = 0; ts < v_scenario.size(); ts++) {
            float y = (float)MathUtils::interpolate(ref_cdf, v_xbins, cdf_values[ts], true);  // Eq. 1
            v_output[ts] = (y >= 0) ? y : 0;
        }
    }
}

/**
 * Quantile Delta Mapping bias correction based on
 * Tong, Y., Gao, X., Han, Z. et al. Bias correction of temperature and precipitation over China for RCM
 * simulations using the QM and QDM methods. Clim Dyn 57, 1425–1443 (2021). https://doi.org/10.1007/s00382-020-05447-4
 *
 * @param v_output 1D output vector that stores the adjusted time series
 * @param v_reference 1D reference time series (control period)
 * @param v_control 1D modeled time series (control period)
 * @param v_scenario 1D time series to adjust (scenario period)
 * @param settings struct with attributes:
 *      double max_scaling_factor,
 *      unsigned n_quantiles,
 *      bool interval31_scaling,
 *      std::string kind
 *
 * Add (+):
 *  (1.1) $\varepsilon(t) = F^{(t)}_{sim,p}\left[T_{sim,p}(t)\right]$
 *  (1.2) $\T^{*1QDM}_{sim,p}(t) = F^{-1}_{obs,h}\left[\varepsilon(t)\right]$
 *  (1.3) $\Delta(t) = F^{-1}_{sim,p}(t)\left[\varepsilon(t)\right] = T_{sim,p}(t)-F^{-1}_{sim,h} \left\{F^{}_{sim,p}(t)\left[T_{sim,p}(t)\right]\right\}$
 *  (1.4) $T^{*QDM}_{sim,p}(t) = T^{*1QDM}_{sim,p}(t) + \Delta(t)$
 *
 * Mult (*):
 *   (2.1) --//-- (same as 1.1)
 *   (2.2) --//-- (same as 1.2)
 *   (2.3) \Delta(i) & = \frac{F^{-1}_{sim,p}\left[\varepsilon(i)\right] }{ F^{-1}_{sim,h}\left[\varepsilon(i)\right] } \\
 *                   & = \frac{X_{sim,p}(i) }{ F^{-1}_{sim,h}\left\{F^{}_{sim,p}\left[X_{sim,p}(i)\right]\right\} }
 *   (2.4) X^{*QDM}_{sim,p}(i) = X^{QDM(1)}_{sim,p}(i) \cdot \Delta(i)
 */
void CMethods::Quantile_Delta_Mapping(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    AdjustmentSettings& settings
) {
    const bool isAdd = (settings.kind == "add" || settings.kind == "+") ? true : false;

    if (!isAdd && !(settings.kind == "mult" || settings.kind == "*"))
        throw std::runtime_error("Adjustment kind " + settings.kind + " unknown for quantile delta mapping!");

    std::vector<double>
        v_xbins = get_xbins(
            v_reference,
            v_control,
            settings.n_quantiles,
            (isAdd) ? "regular" : "bounded"
        );

    // ? create CDF
    std::vector<int>
        vi_ref_cdf = MathUtils::get_cdf(v_reference, v_xbins),
        vi_contr_cdf = MathUtils::get_cdf(v_control, v_xbins),
        vi_scen_cdf = MathUtils::get_cdf(v_scenario, v_xbins);

    // ? change to double
    std::vector<double>
        ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
        contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end()),
        scen_cdf(vi_scen_cdf.begin(), vi_scen_cdf.end());

    std::vector<double> epsilon;
    for (unsigned ts = 0; ts < v_scenario.size(); ts++)
        epsilon.push_back(MathUtils::interpolate(v_xbins, scen_cdf, v_scenario[ts], false));  // Eq. 1.1

    std::vector<double> QDM1;
    // ? insert simulated values into inverse CDF of observed
    for (unsigned ts = 0; ts < v_scenario.size(); ts++)
        QDM1.push_back(MathUtils::interpolate(ref_cdf, v_xbins, epsilon[ts], false));  // Eq. 1.2

    // ? Invert, insert in inversed CDF and return
    if (isAdd) {
        for (unsigned ts = 0; ts < v_scenario.size(); ts++)
            v_output[ts] = (float)(QDM1[ts] + v_scenario[ts] - MathUtils::interpolate(contr_cdf, v_xbins, epsilon[ts], false));  // Eq. 1.3f.

    } else {
        // clang-format off
        for (unsigned ts = 0; ts < v_scenario.size(); ts++) {

            v_output[ts] = QDM1[ts] * MathUtils::ensure_devidable(
                (double) v_scenario[ts],
                MathUtils::interpolate(contr_cdf, v_xbins, epsilon[ts], false),
                settings.max_scaling_factor
            ); // Eq. 2.3f.
        }
        // clang-format on
    }
}
