// -*- lsst-c++ -*-
/**
 * @file TestMathUtils.cxx
 * @brief Implements the unit tests of the MathUtils class
 * @author Benjamin Thomas Schwertfeger
 * @email: contact@b-schwertfeger.de
 * @link https://github.com/btschwertfeger/BiasAdjustCXX
 *
 *  * Copyright (C) 2023 Benjamin Thomas Schwertfeger
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
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <cmath>
#include <vector>

#include "MathUtils.hxx"
#include "gtest/gtest.h"

namespace TestBiasAdjustCXX {
namespace MathUtils {
namespace {

// The fixture for testing class MathUtills.
class TestMathUtils : public ::testing::Test {
   protected:
    TestMathUtils() : sbuf{nullptr} {
    }

    ~TestMathUtils() override {
    }

    void SetUp() override {
        sbuf = std::cout.rdbuf();
        std::cout.rdbuf(buffer.rdbuf());

        v = std::vector<float>({1.0, 0.0, -1.0, 2.0, 0.0, -2.0});
        w = std::vector<float>({0.0, 1.0, 0.0, -2.0, -1.0, 0.0});
        x = std::vector<float>({0.0, 0.0, 0.0, 0.0, 0.0, 0.0});
        y = std::vector<float>({1.0, 1.5, 2.0, 2.5, 3.0, 3.5});
        z = std::vector<float>({-1.0, -1.5, -2.0, -2.5, -3.0, -3.5});
    }

    void TearDown() override {
        std::cout.rdbuf(sbuf);
        sbuf = nullptr;
    }

    std::stringstream buffer{};
    std::streambuf* sbuf;

    std::vector<float> v;
    std::vector<float> w;
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
};

// Test if available methods have not changed
TEST_F(TestMathUtils, CheckAvailableMethods) {
    EXPECT_EQ(::MathUtils::available_methods, std::vector<std::string>({"rmse", "mbe", "ioa", "corr", "sd", "var", "mean"}));
}

// Test if available methods that require one data set have not changed
TEST_F(TestMathUtils, CheckMethodsThatRequireOneDataset) {
    EXPECT_EQ(::MathUtils::requires_1_ds, std::vector<std::string>({"sd", "var", "mean"}));
}

// Test if available methods that require tow data sets have not changed
TEST_F(TestMathUtils, CheckMethodsThatRequireTwoDatasets) {
    EXPECT_EQ(::MathUtils::requires_2_ds, std::vector<std::string>({"rmse", "mbe", "ioa", "corr"}));
}

// Test of the correlation coefficient function
TEST_F(TestMathUtils, CheckCorrelationCoefficient) {
    EXPECT_EQ(std::isnan(::MathUtils::correlation_coefficient(x, x)), true);
    EXPECT_EQ(std::isnan(::MathUtils::correlation_coefficient(x, z)), true);
    EXPECT_EQ(std::isnan(::MathUtils::correlation_coefficient(x, w)), true);
    ASSERT_DOUBLE_EQ(::MathUtils::correlation_coefficient(v, v), 1);
    ASSERT_DOUBLE_EQ(::MathUtils::correlation_coefficient(z, v), 0.45355736761107268);
}

// Test of the RMSE function
TEST_F(TestMathUtils, CheckRootMeanSquareError) {
    ASSERT_DOUBLE_EQ(::MathUtils::rmse(x, x), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::rmse(x, y), 2.4065881796989417);
    ASSERT_DOUBLE_EQ(::MathUtils::rmse(x, z), 2.4065881796989417);
}

// Test of the Mean Bias Error function
TEST_F(TestMathUtils, CheckMeanBiasError) {
    ASSERT_DOUBLE_EQ(::MathUtils::mbe(x, x), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::mbe(x, y), 2.25);
    ASSERT_DOUBLE_EQ(::MathUtils::mbe(y, z), -4.5);
}

// Test of the Index of Agreement function
TEST_F(TestMathUtils, CheckIndexOfAgreement) {
    ASSERT_DOUBLE_EQ(std::isnan(::MathUtils::ioa(x, x)), true);
    // ASSERT_DOUBLE_EQ(::MathUtils::ioa(x, y), double(0));
    // ASSERT_DOUBLE_EQ(::MathUtils::ioa(y, z), 0.18594436310395313);
}

// Test of the Standard Deviation function
TEST_F(TestMathUtils, CheckStandardDeviation) {
    ASSERT_DOUBLE_EQ(::MathUtils::sd(x), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::sd(y), 0.8539125638299665);
    ASSERT_DOUBLE_EQ(::MathUtils::sd(z), 0.8539125638299665);
}

// Test to check the Mean (float)
TEST_F(TestMathUtils, CheckFloatMean) {
    ASSERT_FLOAT_EQ(::MathUtils::mean(v), 0);
    ASSERT_FLOAT_EQ(::MathUtils::mean(w), -0.33333333333333331);
    ASSERT_FLOAT_EQ(::MathUtils::mean(x), 0);
    ASSERT_FLOAT_EQ(::MathUtils::mean(y), 2.25);
    ASSERT_FLOAT_EQ(::MathUtils::mean(z), -2.25);
}

// Test to check the Mean (double)
TEST_F(TestMathUtils, CheckDoubleMean) {
    std::vector<double> a(v.begin(), v.end());
    std::vector<double> b(w.begin(), w.end());
    std::vector<double> c(x.begin(), x.end());
    std::vector<double> d(y.begin(), y.end());
    std::vector<double> e(z.begin(), z.end());
    ASSERT_DOUBLE_EQ(::MathUtils::mean(a), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::mean(b), -0.33333333333333331);
    ASSERT_DOUBLE_EQ(::MathUtils::mean(c), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::mean(d), 2.25);
    ASSERT_DOUBLE_EQ(::MathUtils::mean(e), -2.25);
}

// Test to check the Median (float)
TEST_F(TestMathUtils, CheckFloatMedian) {
    ASSERT_FLOAT_EQ(::MathUtils::median(v), 0);
    ASSERT_FLOAT_EQ(::MathUtils::median(w), 0);
    ASSERT_FLOAT_EQ(::MathUtils::median(x), 0);
    ASSERT_FLOAT_EQ(::MathUtils::median(y), 2.5);
    ASSERT_FLOAT_EQ(::MathUtils::median(z), -2);
}

// Test to check the Median (double)
TEST_F(TestMathUtils, CheckDoubleMedian) {
    std::vector<double> a(v.begin(), v.end());
    std::vector<double> b(w.begin(), w.end());
    std::vector<double> c(x.begin(), x.end());
    std::vector<double> d(y.begin(), y.end());
    std::vector<double> e(z.begin(), z.end());
    ASSERT_DOUBLE_EQ(::MathUtils::median(a), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::median(b), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::median(c), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::median(d), 2.5);
    ASSERT_DOUBLE_EQ(::MathUtils::median(e), -2);
}

// Test to get the probability density function
TEST_F(TestMathUtils, CheckProbabilityDensityFunction) {
    std::vector<double>
        bins1({-5, 0, 5}),
        bins2({-10, -5, -2.5, 0, 2.5, 5, 10});
    std::vector<int> target({2, 4});

    std::vector<int> pdf = ::MathUtils::get_pdf(v, bins1);
    ASSERT_EQ(pdf.size(), bins1.size() - 1);

    for (unsigned i = 0; i < bins1.size() - 1; i++)
        ASSERT_EQ(pdf[i], target[i]);
}

// Test to get the cumulative distribution function
TEST_F(TestMathUtils, CheckCumulativeDistributionFunction) {
    std::vector<double>
        bins1({-5, 0, 5}),
        bins2({-10, -5, -2.5, 0, 2.5, 5, 10});
    std::vector<int> target({0, 2, 6});

    std::vector<int> cdf = ::MathUtils::get_cdf(v, bins1);
    ASSERT_EQ(cdf.size(), bins1.size());

    for (unsigned i = 0; i < bins1.size() - 1; i++)
        ASSERT_EQ(cdf[i], target[i]);
}
// Test the linear inpterpolation
TEST_F(TestMathUtils, CheckLinearInterpolation) {
    std::vector<double> targets({2, -1.5, -3, 12, 3, -9});
    for (unsigned i = 0; i < v.size(); i++)
        ASSERT_EQ(targets[i], ::MathUtils::lerp(v[i], w[i], z[i]));
}

// Test linear interpolation of a value - 2-dimensional
TEST_F(TestMathUtils, CheckLinearInterpolation2d) {
    std::vector<double>
        xData({1.12, 1.1456, 1.234, 12.345, 13.456, 14.5678}),
        yData({0.1, 0.5, -12, 1.2245, 17.98, 25.98}),
        x({13, -1.223, -3.23, 3.33, 5.44, 0.9}),
        targets_no_extrapolation({11.102849714462536, 0.10000000149011612, 0.10000000149011612, -9.5053053412748589, -6.9939476845575568, 0.10000000149011612}),
        targets_with_extrapolation({11.102849714462536, -36.509437637865666, -67.868866230547567, -9.5053053412748589, -6.9939476845575568, -3.3375059476496061});

    for (unsigned i = 0; i < x.size(); i++) {
        ASSERT_EQ(targets_no_extrapolation[i], ::MathUtils::interpolate(xData, yData, x[i], false));
        ASSERT_EQ(targets_with_extrapolation[i], ::MathUtils::interpolate(xData, yData, x[i], true));
    }
}

TEST_F(TestMathUtils, CheckEnsureDevidable) {
    ASSERT_EQ(::MathUtils::ensure_devidable((double)5, (double)5, 10), 1);
    ASSERT_EQ(::MathUtils::ensure_devidable((double)0, (double)5, 10), 0);
    ASSERT_EQ(::MathUtils::ensure_devidable((double)5, (double)0, 10), 50);
    ASSERT_EQ(::MathUtils::ensure_devidable((float)5, (float)5, 10), 1);
    ASSERT_EQ(::MathUtils::ensure_devidable((float)0, (float)5, 10), 0);
    ASSERT_EQ(::MathUtils::ensure_devidable((float)5, (float)0, 10), 50);
}
}  // namespace
}  // namespace MathUtils
}  // namespace TestBiasAdjustCXX
