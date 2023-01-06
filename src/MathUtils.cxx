// -*- lsst-c++ -*-

/**
 * @file MathUtils.cxx
 * @brief Implementation of the MathUtils class
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

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Includes and Namespaces
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include "MathUtils.hxx"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Definitions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

std::vector<std::string> MathUtils::available_methods = {"rmse", "mbe", "ioa", "corr", "sd", "var", "mean"};
std::vector<std::string> MathUtils::requires_1_ds = {"sd", "var", "mean"};
std::vector<std::string> MathUtils::requires_2_ds = {"rmse", "mbe", "ioa", "corr"};

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Object management
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

MathUtils::MathUtils() {}
MathUtils::~MathUtils() {}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Functions
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/**
 * Returns the pointer to a function based on `name`
 *
 * @param name name of the desired funtion
 * @return point to desired funciton
 */
Func_one MathUtils::get_method_for_1_ds(std::string name) {
    if (name == "sd")
        return sd;
    else if (name == "var")
        return variance;
    if (name == "mean")
        return mean;
    else
        return NULL;
}
/**
 * Returns the pointer to a function based on `name`
 *
 * @param name name of the desired funtion
 * @return point to desired funciton
 */
Func_two MathUtils::get_method_for_2_ds(std::string name) {
    if (name == "rmse")
        return rmse;
    else if (name == "mbe")
        return mbe;
    else if (name == "corr")
        return correlation_coefficient;
    else if (name == "ioa")
        return ioa;
    else
        return NULL;
}

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Methods
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

/** Returns the correlation coefficient for given `x` and `y` vectors
 *
 * References:
 *  - https://www.geeksforgeeks.org/program-find-correlation-coefficient/
 *
 *  - AgriMetSoft (2019). Online Calculators. Available on: https://agrimetsoft.com/calculators/R-squared%20correlation
 *    $r^{2} = \frac{
 *          n(\sum(xy)) - \sum(x)\sum(y)
 *      }{
 *          \sqrt{
 *              \left[n\sum{x^{2}} - \sum(x)^{2}\right] \left[n\sum{y^{2}} - (\sum{y})^{2}\right]
 *          }
 *      }$
 *
 * @param x 1D reference time series/vector
 * @param y 1D prediction time series/vector
 * @return correlation coefficient
 */
double MathUtils::correlation_coefficient(std::vector<float>& x, std::vector<float>& y) {
    if (x.size() != y.size()) throw std::runtime_error("Cannot calculate correlation coefficient of vectors with different size.");
    double
        sum_X = 0,
        sum_Y = 0,
        sum_XY = 0,
        squareSum_X = 0,
        squareSum_Y = 0;

    for (unsigned ts = 0; ts < x.size(); ts++) {
        sum_X = sum_X + x[ts];            // sum of elements of array x.
        sum_Y = sum_Y + y[ts];            // sum of elements of array y.
        sum_XY = sum_XY + x[ts] * y[ts];  // sum of x[i] * y[i].
        // sum of square of array elements.
        squareSum_X = squareSum_X + x[ts] * x[ts];
        squareSum_Y = squareSum_Y + y[ts] * y[ts];
    }
    return (x.size() * sum_XY - sum_X * sum_Y) / sqrt((x.size() * squareSum_X - sum_X * sum_X) * (x.size() * squareSum_Y - sum_Y * sum_Y));
}

/** Returns the Root Mean Square Error
 *   RMSE = \sqrt{
 *             \frac{
 *                \sum_{i=1}^{n}(T_{y,i} - T_{x,i})^{2}
 *             }{
 *                 n
 *             }
 *         }
 *
 * @param x 1D reference time series/vector
 * @param y 1D prediction time series/vector
 * @return root mean square error
 */
double MathUtils::rmse(std::vector<float>& x, std::vector<float>& y) {
    if (x.size() != y.size()) throw std::runtime_error("Cannot calculate rmse of vectors with different size.");

    double result = 0;
    for (unsigned ts = 0; ts < x.size(); ts++) result += pow(y[ts] - x[ts], 2) / (int)x.size();
    return sqrt(result);
}

/** Returns the Mean Bias Error
 *  $MBE = \frac{1}{n} \sum_{i=1}^{n}(T_{y,i} - T_{x,i})$
 *
 * @param x 1D reference time series/vector
 * @param y 1D prediction time series/vector
 * @ return mean bias error
 */
double MathUtils::mbe(std::vector<float>& x, std::vector<float>& y) {
    if (x.size() != y.size()) throw std::runtime_error("Cannot calculate mbe of vectors with different size.");

    double result = 0;
    for (unsigned ts = 0; ts < x.size(); ts++) result += y[ts] - x[ts];
    return result * (double(1.0) / (int)x.size());
}

/** Returns the index of agreement
 *  $d = 1 - \frac{
 *       \sum^{n}_{i=1}(T_{obs,p,i} - T_{sim,p}(i))^{2}
 *      }{
 *       \sum^{n}_{i=1}(| T_{sim,p}(i) - \mu({T_{obs,p}})|+|T_{obs,p}(i) - \mu({T_{obs,p}})|)
 *      }, \hspace{1em}  d\in \{0,1\}
 *  $
 *
 * @param x 1D reference time series/vector
 * @param y 1D prediction time series/vector
 * @return index of agreenemnt
 */
double MathUtils::ioa(std::vector<float>& x, std::vector<float>& y) {
    if (x.size() != y.size()) throw std::runtime_error("Cannot calculate ioa of vectors with different size.");

    double upper = 0, lower = 0;
    const double m = mean(x);
    for (unsigned i = 0; i < x.size(); i++) {
        upper += pow(x[i] - y[i], 2);
        lower += pow(abs(y[i] - m) + abs(x[i] - m), 2);
    }
    return (1 - (upper / lower));
}

/** Returns the variance
 *  $\sigma^{2}(x) = \frac{\sum_{i=1}^{n}(x_{i} - \mu(x)^{2}}{n-1}$
 *
 * @param x 1D vector of interest
 * @return variance of `x`
 */
double MathUtils::variance(std::vector<float>& x) {
    std::vector<double> v(x.size());
    const double m = mean(x);
    for (unsigned i = 0; i < x.size(); i++) v[i] = pow(x[i] - m, 2);
    return mean(v);
}

/** Returns the standard deviation
 *  $\sigma(x) = \sqrt{\frac{\sum_{i=1}^{n}(x_{i}-\mu(x)^{2}}{n-1}}$
 *
 * @param x 1D vector of interest
 * @return standard deviation of `x`
 */
double MathUtils::sd(std::vector<float>& x) {
    return sqrt(variance(x));
}

/** Returns the mean
 *
 * @param a 1D vector of floats/doubles to get the mean from
 * @return mean of `a`
 */
double MathUtils::mean(std::vector<float>& a) {
    double sum = 0;
    for (unsigned i = 0; i < a.size(); i++) sum += a[i];
    return sum / a.size();
}
double MathUtils::mean(std::vector<double>& a) {
    double sum = 0;
    for (unsigned i = 0; i < a.size(); i++) sum += a[i];
    return sum / a.size();
}

/** Returns the median
 *
 * @param a 1D vector of floats/doubles to get the median from
 * @return median of `a`
 */

float MathUtils::median(std::vector<float>& a) {
    std::sort(a.begin(), a.end());
    return a[(int)(a.size() / 2)];
}
double MathUtils::median(std::vector<double>& a) {
    std::sort(a.begin(), a.end());
    return a[(int)(a.size() / 2)];
}

/**
 * Computes the Probabillity Density Function (PDF)
 * $F(x) = P(a \leq x \leq b) = \int_{a}^b f(x)dx \geq 0$
 * where:
 *   $a$ and $b$ are the first resp. last value of `bins`
 *
 * @param arr 1D vector to compute the PDF from
 * @param bins probability boundaries to assign the probabilities
 * @return probability densitiy function of `arr` based on `bins` as integer vector
 */
std::vector<int> MathUtils::get_pdf(std::vector<float>& arr, std::vector<double>& bins) {
    std::vector<int> v_pdf(bins.size() - 1);
    for (unsigned ts = 0; ts < arr.size(); ts++) {
        for (unsigned i = 0; i < v_pdf.size() - 1; i++) {
            if (i == 0 && arr[ts] <= bins[i]) {
                ++v_pdf[i];
                break;
            } else if (arr[ts] >= bins[i] && arr[ts] < bins[i + 1]) {
                ++v_pdf[i];
                break;
            } else if (i == v_pdf.size() - 2 && arr[ts] >= bins[i + 1]) {
                ++v_pdf[i + 1];
                break;
            }
        }
    }
    return v_pdf;
}

/**
 * Computes the Cumulative Distribution Function (CDF)
 *  summed up PDF
 *
 * @param arr 1D vector to compute the CDF from
 * @param bins probability boundaries to assign the probabilities
 * @return cumulative distribution function of `arr` based on `bins` as int vector
 */
std::vector<int> MathUtils::get_cdf(std::vector<float>& arr, std::vector<double>& bins) {
    std::vector<int> v_pdf = MathUtils::get_pdf(arr, bins);
    std::vector<int> v_cdf(v_pdf.size() + 1);
    v_cdf[0] = 0;
    for (unsigned i = 0; i < v_pdf.size(); i++)
        v_cdf[i + 1] = v_cdf[i] + v_pdf[i];
    return v_cdf;
}
/**
 * Linear interpolation between two values
 * $y = a + x \cdot (b - a)$
 *
 * @param a fist value
 * @param b second value
 * @param x value to interpolate between `a` and `b`
 */
double MathUtils::lerp(double a, double b, double x) {
    return a + x * (b - a);
}

/**
 * Returns interpolated value at `x` from parallel vectors ( `xData`, `yData` )
 *  "Assumes that `xData` has at least two elements, is sorted and is strictly monotonic increasing
 *  boolean argument extrapolate determines behaviour beyond ends of array (if needed)"
 *  Reference: https://www.cplusplus.com/forum/general/216928/
 *
 * @param xData increasing vector of double values
 * @param yData y values corresponding to xData
 * @param x value to interpolate
 * @param extrapolate behaviour outside xData range
 * @return the interpolated value
 */
double MathUtils::interpolate(std::vector<double>& xData, std::vector<double>& yData, double x, bool extrapolate) {
    int size = xData.size();

    int i = 0;  // find left end of interval for interpolation
    if (x >= xData[size - 2])
        i = size - 2;  // special case: beyond right end
    else
        while (x > xData[i + 1]) i++;

    float
        xL = xData[i],
        yL = yData[i],
        xR = xData[i + 1],
        yR = yData[i + 1];  // points on either side (unless beyond ends)

    if (!extrapolate) {  // if beyond ends of array and not extrapolating
        if (x < xL) yR = yL;
        if (x > xR) yL = yR;
    }

    double dydx;
    if (xR - xL == 0)
        dydx = 0;
    else
        dydx = (yR - yL) / (xR - xL);  // gradient
    return yL + dydx * (x - xL);       // linear interpolation
}
