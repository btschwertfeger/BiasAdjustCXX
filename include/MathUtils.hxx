// -*- lsst-c++ -*-

/**
 * @file MathUtils.hxx
 * @brief Declaration of the MathUtils class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 */

#ifndef __MathUtils__
#define __MathUtils__

#include <iostream>
#include <vector>

typedef float (*Func_one)(std::vector<float>& x);
typedef float (*Func_two)(std::vector<float>& x, std::vector<float>& y);

class MathUtils {
   public:
    MathUtils();
    ~MathUtils();

    static std::vector<std::string> available_methods;
    static std::vector<std::string> requires_1_ds;
    static std::vector<std::string> requires_2_ds;

    static Func_one get_method_for_1_ds(std::string name);
    static Func_two get_method_for_2_ds(std::string name);

    static float correlation_coefficient(std::vector<float>& x, std::vector<float>& y);
    static float rmse(std::vector<float>& x, std::vector<float>& y);
    static float mbe(std::vector<float>& x, std::vector<float>& y);
    static float ioa(std::vector<float>& x, std::vector<float>& y);
    static float sd(std::vector<float>& x);
    static float variance(std::vector<float>& x);
    static float mean(std::vector<float>& a);
    static double lerp(double a, double b, double x);

    static std::vector<int> get_pdf(std::vector<float>& arr, std::vector<double>& bins);
    static std::vector<int> get_cdf(std::vector<float>& arr, std::vector<double>& bins);
    static double interpolate(std::vector<double>& xData, std::vector<double>& yData, double x, bool extrapolate);

   private:
};

#endif