// -*- lsst-c++ -*-

/**
 * @file CMethods.cpp
 * @brief
 * @author Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 *
 * * Description:

 * * Notes:

 * * Usage:

 */

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Includes and Namespaces
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include "CMethods.hxx"

#include <iostream>
#include <vector>

#include "MyMath.hxx"
#include "math.h"

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Definitions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

std::vector<std::string> CMethods::simple_method_names = {"linear_scaling", "variance_scaling", "delta_method"};
std::vector<std::string> CMethods::quantile_method_names = {"quantile_mapping", "quantile_delta_mapping"};

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Object Management
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

CMethods::CMethods() {}
CMethods::~CMethods() {}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Functions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

CM_Func_ptr_simple CMethods::get_cmethod_simple(std::string method_name) {
    if (method_name == "linear_scaling")
        return Linear_Scaling;
    else if (method_name == "variance_scaling")
        return Variance_Scaling;
    else if (method_name == "delta_method")
        return Delta_Method;
    return NULL;
}
CM_Func_ptr_quantile CMethods::get_cmethod_quantile(std::string method_name) {
    if (method_name == "quantile_mapping")
        return Quantile_Mapping;
    else if (method_name == "quantile_delta_mapping")
        return Quantile_Delta_Mapping;
    return NULL;
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *              Adjustment methods
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * Method to adjust 1 dimensional climate data by the linear scaling method.
 *
 * @param output output array where to save the adjusted data
 * @param reference observation data (control period)
 * @param control modeled data of the control period
 * @param scenario data to adjust
 * @param n_time length of input and output arrays
 * @param kind type of adjustment; additive or multiplicative
 *
 * Add ('+'):
 *  (1.)    $T^{*}_{contr}(d)=T_{contr}(d)+\mu_{m}(T_{obs}(d))-\mu_{m}(T_{contr}(d))$
 *  (2.)    $T^{*}_{scen}(d)=T_{scen}(d)+\mu_{m}(T_{obs}(d))-\mu_{m}(T_{scen}(d))$
 * Mult ('*'):
 *  (3.)    $ T_{contr}^{*}(d) = \mu_{m}(T_{obs}(d))\cdot\left[\frac{T_{contr}(d)}{\mu_{m}(T_{contr}(d))}\right]$
 *  (4.)    $ T_{scen}^{*}(d) = \mu_{m}(T_{obs}(d))\cdot\left[\frac{T_{contr}(d)}{\mu_{m}(T_{scen}(d))}\right]$
 *
 */
void CMethods::Linear_Scaling(float* output, float* reference, float* control, float* scenario, unsigned n_time, std::string kind) {
    const float
        ref_mean = MyMath::mean(reference, n_time),
        contr_mean = MyMath::mean(control, n_time);

    if (kind == "add" || kind == "+") {
        for (unsigned ts = 0; ts < n_time; ts++)
            output[ts] = scenario[ts] + (ref_mean - contr_mean);  // Eq. 1f.
    } else if (kind == "mult" || kind == "*") {
        for (unsigned ts = 0; ts < n_time; ts++)
            output[ts] = scenario[ts] * (ref_mean / contr_mean);  // Eq. 3f.
    } else
        std::runtime_error("Invalid adjustment kind " + kind + "!");
}

/**
 * Method to adjust 1 dimensional climate data by variance scaling method.
 *
 * @param output output array where to save the adjusted data
 * @param reference observation data (control period)
 * @param control modeled data of the control period
 * @param scenario data to adjust
 * @param n_time length of input and output arrays
 * @param kind type of adjustment; additive or multiplicative
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
void CMethods::Variance_Scaling(float* output, float* reference, float* control, float* scenario, unsigned n_time, std::string kind) {
    if (kind == "add" || kind == "+") {
        float
            LS_contr[n_time],
            LS_scen[n_time];

        Linear_Scaling(LS_contr, reference, control, control, n_time, "+");  // Eq. 1
        Linear_Scaling(LS_scen, reference, control, scenario, n_time, "+");  // Eq. 2

        float
            LS_contr_mean = MyMath::mean(LS_contr, n_time),
            LS_scen_mean = MyMath::mean(LS_scen, n_time);

        float
            VS1_contr[n_time],
            VS1_scen[n_time];

        for (unsigned ts = 0; ts < n_time; ts++) {
            VS1_contr[ts] = LS_contr[ts] - LS_contr_mean;  // Eq. 3
            VS1_scen[ts] = LS_scen[ts] - LS_scen_mean;     // Eq. 4
        }

        float
            ref_sd = MyMath::sd(reference, n_time),
            VS1_contr_sd = MyMath::sd(VS1_contr, n_time);

        float VS2_scen[n_time];
        for (unsigned ts = 0; ts < n_time; ts++) {
            VS2_scen[ts] = VS1_scen[ts] * (ref_sd / VS1_contr_sd);  // Eq. 6
            output[ts] = VS2_scen[ts] + LS_scen_mean;               // Eq. 7
        }

    } else if (kind == "mult" || kind == "*")
        std::runtime_error("Multiplicative Variance Scaling Method not implemented!");
    else
        std::runtime_error("Invalid adjustment kind " + kind + "!");
}

/**
 * Method to adjust 1 dimensional climate data by delta method.
 *
 * @param output output array where to save the adjusted data
 * @param reference observation data (control period)
 * @param control modeled data of the control period
 * @param scenario data to adjust
 * @param n_time length of input and output arrays
 * @param kind type of adjustment; additive or multiplicative
 *
 * Add (+):
 *   (1.) $T^{*}_{contr}(d) = T_{contr}(d) + (\mu_{m}(T_{scen}(d)) - \mu_{m}(T_{obs}(d)))$
 * Mult (*):
 *   (2.) $T^{*}_{contr}(d) = T_{contr}(d) \cdot \left[\frac{\mu_{m}(T_{scen}(d))}{\mu_{m}(T_{obs}(d))}\right]$
 *
 */
void CMethods::Delta_Method(float* output, float* reference, float* control, float* scenario, unsigned n_time, std::string kind) {
    const float
        contr_mean = MyMath::mean(control, n_time),
        scen_mean = MyMath::mean(scenario, n_time);

    if (kind == "add" || kind == "+") {
        for (unsigned ts = 0; ts < n_time; ts++)
            output[ts] = reference[ts] + (scen_mean - contr_mean);  // Eq. 1
    } else if (kind == "mult" || kind == "*") {
        for (unsigned ts = 0; ts < n_time; ts++)
            output[ts] = reference[ts] * (scen_mean / contr_mean);  // Eq. 2
    } else
        std::runtime_error("Invalid adjustment kind " + kind + "!");
}

/**
 *
 * @param kind regular or bounded: defines if min is the real minimum or set to 0
 */
std::vector<double> CMethods::get_xbins(float* a, float* b, unsigned n_quantiles, unsigned length, std::string kind) {
    if (kind == "regular") {
        const double
            a_max = *std::max_element(a, a + length),
            a_min = *std::min_element(a, a + length),
            b_max = *std::max_element(b, b + length),
            b_min = *std::min_element(b, b + length);

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
            a_max = *std::max_element(a, a + length),
            b_max = *std::max_element(b, b + length);
        const double global_max = std::max(a_max, b_max);
        const double wide = global_max / n_quantiles;

        std::vector<double> v_xbins(0);
        v_xbins.push_back((double)0);
        while (v_xbins[v_xbins.size() - 1] < global_max)
            v_xbins.push_back(v_xbins[v_xbins.size() - 1] + wide);
        return v_xbins;
    } else {
        std::runtime_error("Unknown kind for get_xbins-function.");
        return std::vector<double>(0);  // just to avoid warnings ...
    }
}
/**
 * Quantile Mapping bias correction based on
 * Tong, Y., Gao, X., Han, Z. et al. Bias correction of temperature and precipitation over China for RCM
 * simulations using the QM and QDM methods. Clim Dyn 57, 1425–1443 (2021). https://doi.org/10.1007/s00382-020-05447-4
 *
 * @param output output array where to save the adjusted data
 * @param reference observation data (control period)
 * @param control modeled data of the control period
 * @param scenario data to adjust
 * @param n_time length of input and output arrays
 * @param kind type of adjustment; additive or multiplicative
 * @param n_quantiles number of quantiles to use
 *
 * (add):
 *      (1.) $T^{*QM}_{sim,p}(d)=F^{-1}_{obs,h} \left\{F_{sim,h}\left[T_{sim,p}(d)\right]\right\}$
 * (mult):
 *      same but experimental ...
 */
void CMethods::Quantile_Mapping(float* output, float* reference, float* control, float* scenario, unsigned n_time, std::string kind, unsigned n_quantiles) {
    if (kind == "add" || kind == "+") {
        std::vector<double> v_xbins = get_xbins(reference, control, n_quantiles, n_time, "regular");

        // ? create CDF
        std::vector<int>
            vi_ref_cdf = MyMath::get_cdf(reference, v_xbins, n_time),
            vi_contr_cdf = MyMath::get_cdf(control, v_xbins, n_time);

        std::vector<double>
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end());

        // ? Interpolate
        std::vector<double> cdf_values;
        for (unsigned ts = 0; ts < n_time; ts++)
            cdf_values.push_back(MyMath::interpolate(v_xbins, contr_cdf, (double)scenario[ts], false));

        // ? Invert in inversed CDF and return
        for (unsigned ts = 0; ts < n_time; ts++)
            output[ts] = (float)MyMath::interpolate(ref_cdf, v_xbins, cdf_values[ts], false);

    } else if (kind == "mult" || kind == "*") {
        std::vector<double> v_xbins = get_xbins(reference, control, n_quantiles, n_time, "bounded");

        // ? create CDF
        std::vector<int>
            vi_ref_cdf = MyMath::get_cdf(reference, v_xbins, n_time),
            vi_contr_cdf = MyMath::get_cdf(control, v_xbins, n_time);

        std::vector<double>
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end());

        // ? Interpolate
        std::vector<double> cdf_values;
        for (unsigned ts = 0; ts < n_time; ts++) {
            double y = MyMath::interpolate(v_xbins, contr_cdf, (double)scenario[ts], true);
            cdf_values.push_back((y >= 0) ? y : 0);
        }

        // ? Invert in inversed CDF and return
        for (unsigned ts = 0; ts < n_time; ts++) {
            float y = (float)MyMath::interpolate(ref_cdf, v_xbins, cdf_values[ts], true);
            output[ts] = (y >= 0) ? y : 0;
        }

    } else
        std::runtime_error("Adjustment kind " + kind + " unknown for Quantile Mapping!");
}

/**
 * Quantile Delta Mapping bias correction based on
 * Tong, Y., Gao, X., Han, Z. et al. Bias correction of temperature and precipitation over China for RCM
 * simulations using the QM and QDM methods. Clim Dyn 57, 1425–1443 (2021). https://doi.org/10.1007/s00382-020-05447-4
 *
 * @param output output array where to save the adjusted data
 * @param reference observation data (control period)
 * @param control modeled data of the control period
 * @param scenario data to adjust
 * @param n_time length of input and output arrays
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
 *   (2.3) \Delta(i) & = \frac{ F^{-1}_{sim,p}\left[\varepsilon(i)\right] }{ F^{-1}_{sim,h}\left[\varepsilon(i)\right] } \\[1pt]
 *                   & = \frac{ X_{sim,p}(i) }{ F^{-1}_{sim,h}\left\{F^{}_{sim,p}\left[X_{sim,p}(i)\right]\right\} }
 *   (2.4) X^{*QDM}_{sim,p}(i) = X^{QDM(1)}_{sim,p}(i) \cdot \Delta(i)
 */
void CMethods::Quantile_Delta_Mapping(float* output, float* reference, float* control, float* scenario, unsigned n_time, std::string kind, unsigned n_quantiles) {
    if (kind == "add" || kind == "+") {
        std::vector<double> v_xbins = get_xbins(reference, control, n_quantiles, n_time, "regular");

        // ? create CDF
        std::vector<int>
            vi_ref_cdf = MyMath::get_cdf(reference, v_xbins, n_time),
            vi_contr_cdf = MyMath::get_cdf(control, v_xbins, n_time),
            vi_scen_cdf = MyMath::get_cdf(scenario, v_xbins, n_time);

        std::vector<double>
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end()),
            scen_cdf(vi_scen_cdf.begin(), vi_scen_cdf.end());

        std::vector<double> cdf_values;
        for (unsigned ts = 0; ts < n_time; ts++)
            cdf_values.push_back(MyMath::interpolate(v_xbins, scen_cdf, scenario[ts], false));

        std::vector<double> QDM1;  // insert simulated values into inverse cdf of observed
        for (unsigned ts = 0; ts < n_time; ts++) QDM1.push_back(MyMath::interpolate(ref_cdf, v_xbins, cdf_values[ts], false));

        // ? Invert, insert in inversed CDF and return
        for (unsigned ts = 0; ts < n_time; ts++)
            output[ts] = (float)(QDM1[ts] + scenario[ts] - MyMath::interpolate(contr_cdf, v_xbins, cdf_values[ts], false));  // Eq. 2f.

    } else if (kind == "mult" || kind == "*") {
        std::vector<double> v_xbins = get_xbins(reference, control, n_quantiles, n_time, "bounded");

        // ? create CDF
        std::vector<int>
            vi_ref_cdf = MyMath::get_cdf(reference, v_xbins, n_time),
            vi_contr_cdf = MyMath::get_cdf(control, v_xbins, n_time),
            vi_scen_cdf = MyMath::get_cdf(scenario, v_xbins, n_time);

        std::vector<double>
            ref_cdf(vi_ref_cdf.begin(), vi_ref_cdf.end()),
            contr_cdf(vi_contr_cdf.begin(), vi_contr_cdf.end()),
            scen_cdf(vi_scen_cdf.begin(), vi_scen_cdf.end());

        std::vector<double> cdf_values;
        for (unsigned ts = 0; ts < n_time; ts++)
            cdf_values.push_back(MyMath::interpolate(v_xbins, scen_cdf, scenario[ts], false));

        std::vector<double> QDM1;  // insert simulated values into inverse cdf of observed
        for (unsigned ts = 0; ts < n_time; ts++) QDM1.push_back(MyMath::interpolate(ref_cdf, v_xbins, cdf_values[ts], false));

        // ? Invert, insert in inversed CDF and return
        for (unsigned ts = 0; ts < n_time; ts++)
            output[ts] = (float)(QDM1[ts] * (scenario[ts] / MyMath::interpolate(contr_cdf, v_xbins, cdf_values[ts], false)));  // Eq. 2.3f.
    } else
        std::runtime_error("Adjustment kind " + kind + " unknown for Quantile Delta Mapping!");
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        E O F
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */
