// -*- lsst-c++ -*-

/**
 * @file MyMath.h
 * @brief declaration of the MyMath class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 */

#ifndef __MYMATH__
#define __MYMATH__

#include <iostream>

typedef float (*Func_one)(float* x, unsigned n);
typedef float (*Func_two)(float* x, float* y, unsigned n);

class MyMath {
   public:
    MyMath();
    ~MyMath();

    static std::vector<std::string> available_methods;
    static std::vector<std::string> requires_1_ds;
    static std::vector<std::string> requires_2_ds;

    static Func_one get_method_for_1_ds(std::string name);
    static Func_two get_method_for_2_ds(std::string name);

    static float correlation_coefficient(float* x, float* y, unsigned n);
    static float rmse(float* x, float* y, unsigned n);
    static float mbe(float* x, float* y, unsigned n);
    static float ioa(float* x, float* y, unsigned n);
    static float sd(float* x, unsigned n);
    static float variance(float* x, unsigned n);
    static float mean(float* a, unsigned n);
    static double lerp(double a, double b, double x);

    static std::vector<int> get_pdf(float* arr, std::vector<double> bins, unsigned length);
    static std::vector<int> get_cdf(float* arr, std::vector<double> bins, unsigned length);
    static double interp(std::vector<double>& xData, std::vector<double>& yData, double x);
    static double interpolate(std::vector<double>& xData, std::vector<double>& yData, double x, bool extrapolate);
};

#endif