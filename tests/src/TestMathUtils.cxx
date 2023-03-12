// -*- lsst-c++ -*-
/**
 * @file TestMathUtils.cxx
 * @brief
 * @author Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/BiasAdjustCXX
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
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    EXPECT_EQ(isnan(::MathUtils::correlation_coefficient(x, x)), true);
    EXPECT_EQ(isnan(::MathUtils::correlation_coefficient(x, z)), true);
    EXPECT_EQ(isnan(::MathUtils::correlation_coefficient(x, w)), true);
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
    ASSERT_DOUBLE_EQ(isnan(::MathUtils::ioa(x, x)), true);
    ASSERT_DOUBLE_EQ(::MathUtils::ioa(x, y), 0);
    ASSERT_DOUBLE_EQ(::MathUtils::ioa(y, z), 0.18594436310395313);
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
    std::vector<double> bins1({-5, 0, 5});
    std::vector<double> bins2({-10, -5, -2.5, 0, 2.5, 5, 10});

    // ASSERT_THAT(::MathUtils::get_pdf(x, bins1), {2, 4});
}

}  // namespace
}  // namespace MathUtils
}  // namespace TestBiasAdjustCXX
