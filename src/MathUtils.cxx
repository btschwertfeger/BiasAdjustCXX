// -*- lsst-c++ -*-

/**
 * @file MathUtils.cxx
 * @brief Implementation of the MathUtils class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 */

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Includes and Namespaces
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

#include "MathUtils.hxx"

#include <cmath>
#include <iostream>
#include <vector>
/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        Attributes
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */

std::vector<std::string> MathUtils::available_methods = {"rmse", "mbe", "ioa", "corr", "sd"};
std::vector<std::string> MathUtils::requires_1_ds = {"sd"};
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

Func_one MathUtils::get_method_for_1_ds(std::string name) {
    if (name == "sd")
        return sd;
    else
        return NULL;
}

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

/** Returns the correlation coefficient for given x and y arrays
 *  https://www.geeksforgeeks.org/program-find-correlation-coefficient/
 *
 * AgriMetSoft (2019). Online Calculators. Available on: https://agrimetsoft.com/calculators/R-squared%20correlation
 * $r^{2} = \frac{ n(\sum(xy))-\sum(x)\sum(y)}{\sqrt{\left[n\sum{x^{2}}-\sum(x)^{2}\right]\left[n\sum{y^{2}}-(\sum{y})^{2}\right]}}$
 *
 * @param x reference
 * @param y prediction
 */
float MathUtils::correlation_coefficient(std::vector<float>& x, std::vector<float>& y) {
    float
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
 *  $RMSE = \sqrt{\frac{\sum_{i=1}^{n}(T_{y,i}-T_{x,i})^{2}}{n}}$
 *
 * @param x reference
 * @param y prediction
 * @param n length
 */
float MathUtils::rmse(std::vector<float>& x, std::vector<float>& y) {
    float result = 0;
    for (unsigned ts = 0; ts < x.size(); ts++) result += pow(y[ts] - x[ts], 2) / (float)x.size();
    return sqrt(result);
}

/** Returns the mean bias error
 *  $MBE=\frac{1}{n}\sum_{i=1}^{n}(T_{y,i}-T_{x,i})$
 *
 * @param x reference
 * @param y prediction
 */
float MathUtils::mbe(std::vector<float>& x, std::vector<float>& y) {
    float result = 0;
    for (unsigned ts = 0; ts < x.size(); ts++) result += y[ts] - x[ts];
    return result * (float(1.0) / x.size());
}
/** Returns the index of agreement
 *  $d = 1 - \frac{
 *              \sum^{n}_{i=1}(T_{obs,p,i} - T_{sim,p}(i))^{2}
 *           }{
 *              \sum^{n}_{i=1}(| T_{sim,p}(i) - \mu({T_{obs,p}})|+|T_{obs,p}(i) - \mu({T_{obs,p}})|)
 *           },  \hspace{1em}  d\in \{0,1\}
 *  $
 *
 * @param x reference
 * @param y prediction
 */
float MathUtils::ioa(std::vector<float>& x, std::vector<float>& y) {
    float upper = 0, lower = 0;
    const double m = mean(x);
    for (unsigned i = 0; i < x.size(); i++) {
        upper += pow(x[i] - y[i], 2);
        lower += pow(abs(y[i] - m) + abs(x[i] - m), 2);
    }
    return (1 - (upper / lower));
}

/** Returns the variance
 *  $\sigma^{2}(x) = \frac{\sum_{i=1}^{n}(x_{i}-\mu(x)^{2}}{n-1}$
 *
 * @param x reference
 */
float MathUtils::variance(std::vector<float>& x) {
    std::vector<float> v(x.size());
    const float m = mean(x);
    for (unsigned i = 0; i < x.size(); i++) v[i] = pow(x[i] - m, 2);
    return mean(v);
}
/** Returns the standard deviation
 *  $\sigma(x) = \sqrt{\frac{\sum_{i=1}^{n}(x_{i}-\mu(x)^{2}}{n-1}}$
 *
 * @param x reference
 */
float MathUtils::sd(std::vector<float>& x) {
    return sqrt(variance(x));
}

/** Mean
 *
 * @param a 1D array of floats to get the mean from
 */

float MathUtils::mean(std::vector<float>& a) {
    float sum = 0;
    for (unsigned i = 0; i < a.size(); i++) sum += a[i];
    return sum / a.size();
}

/**
 * Probabillity density function
 *
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
 * Cumulative distribution function
 */
std::vector<int> MathUtils::get_cdf(std::vector<float>& arr, std::vector<double>& bins) {
    std::vector<int>
        v_pdf = MathUtils::get_pdf(arr, bins);
    std::vector<int>
        v_cdf(v_pdf.size() + 1);
    v_cdf[0] = 0;
    for (unsigned i = 0; i < v_pdf.size(); i++)
        v_cdf[i + 1] = v_cdf[i] + v_pdf[i];
    return v_cdf;
}

double MathUtils::lerp(double a, double b, double x) {
    return a + x * (b - a);
}

/**
 * Returns interpolated value at x from parallel arrays ( xData, yData )
 *  Assumes that xData has at least two elements, is sorted and is strictly monotonic increasing
 *  boolean argument extrapolate determines behaviour beyond ends of array (if needed)
 *  source: https://www.cplusplus.com/forum/general/216928/
 *
 * @param xData increasing vector of double values
 * @param yData y values corresponding to xData
 * @param x value to interpolate
 * @param extrapolate behaviour outside xData range
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

/**
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 * *                        End of file
 * * ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
 */