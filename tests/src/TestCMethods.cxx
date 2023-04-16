// -*- lsst-c++ -*-
/**
 * @file TestCMethods.cxx
 * @brief File containing unit tests for the CMethods class
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

#include "CMethods.hxx"
#include "gtest/gtest.h"

namespace TestBiasAdjustCXX {
namespace CMethods {
namespace {

// The fixture for testing class CMethods.
class TestCMethods : public ::testing::Test {
   protected:
    TestCMethods() {
    }

    ~TestCMethods() override {
    }

    void SetUp() override {
        // setup fake temperature and precipitation data for latitude = 52°N

        // i.e., 30 years with 365 days each (without leap)
        std::vector<float> temperature(days);
        std::vector<float> precipitation(days);

        for (unsigned ts = 0; ts < days; ts++) {
            temperature[ts] = lat * cos(2 * pi * (ts % 365) / 365 + (2 * random_sample(0, temperature.size())) + kelvin + 0.1 * ((temperature.size() - 1) / 365));
            precipitation[ts] = pow(cos(2 * pi * (ts % 365) / 365), 2) * random_sample(0, precipitation.size());
        }

        auto max_pr = *std::max_element(std::begin(precipitation), std::end(precipitation));
        const int c = days_without_rain * years;

        reference_temp = new std::vector<float>(days);
        target_temp = new std::vector<float>(days);
        control_temp = new std::vector<float>(days);
        scenario_temp = new std::vector<float>(days);

        reference_prec = new std::vector<float>(days);
        target_prec = new std::vector<float>(days);
        control_prec = new std::vector<float>(days);
        scenario_prec = new std::vector<float>(days);

        for (unsigned ts = 0; ts < days; ts++) {
            float prec = precipitation[ts] * 0.0004 / max_pr;  // scale down

            if (random_sample(0, days) < days_without_rain)
                prec = 0;

            reference_prec->at(ts) = prec;
            target_prec->at(ts) = prec * 1.02;
            control_prec->at(ts) = prec * 0.98;
            scenario_prec->at(ts) = prec * 0.09;

            reference_temp->at(ts) = temperature[ts];
            target_temp->at(ts) = temperature[ts] + 2.1;
            control_temp->at(ts) = temperature[ts] - 1.1;
            scenario_temp->at(ts) = temperature[ts] + 0.9;
        }
    }

    /**
     * Retuns a random number between min and max
     */
    int random_sample(int min, int max) {
        int n = max - min + 1;
        int remainder = RAND_MAX % n;
        int x;
        do {
            x = rand();
        } while (x >= RAND_MAX - remainder);
        return min + x % n;
    }

    /** Returns the Mean Bias Error (taken from MathUtils.cxx)
     *  $MBE = \frac{1}{n} \sum_{i=1}^{n}(T_{y,i} - T_{x,i})$
     *
     * @param x 1D reference time series/vector
     * @param y 1D prediction time series/vector
     * @ return mean bias error
     */
    double mbe(std::vector<float> &x, std::vector<float> &y) {
        if (x.size() != y.size()) throw std::runtime_error("Cannot calculate mbe of vectors with different size.");

        double result = 0;
        for (unsigned ts = 0; ts < x.size(); ts++) result += y[ts] - x[ts];
        return result * (double(1.0) / (int)x.size());
    }

    void TearDown() override {
        delete reference_temp;
        delete target_temp;
        delete control_temp;
        delete scenario_temp;
        delete reference_prec;
        delete target_prec;
        delete control_prec;
        delete scenario_prec;
    }
    int
        lat = 52,
        days_without_rain = 239,
        days = 10950,
        years = 30;

    double
        kelvin = 273.15,
        pi = atan(1) * 4;

    std::vector<float> *reference_temp;
    std::vector<float> *reference_prec;
    std::vector<float> *target_temp;
    std::vector<float> *target_prec;
    std::vector<float> *control_temp;
    std::vector<float> *control_prec;
    std::vector<float> *scenario_temp;
    std::vector<float> *scenario_prec;
};

// Test the calculation of the long-term day of year
TEST_F(TestCMethods, CheckGetLongTermDayOfYear) {
    std::vector<std::vector<float>> result = ::CMethods::get_long_term_dayofyear(
        *reference_temp
    );

    ASSERT_EQ(result.size(), 365);
    // maybe add some other checks but that would mock the whole method.
}

// Test the adjusted scaling factor method
TEST_F(TestCMethods, CheckGetAdjustedScalingFactor) {
    ASSERT_EQ(::CMethods::get_adjusted_scaling_factor(10, 5), 5);
    ASSERT_EQ(::CMethods::get_adjusted_scaling_factor(10, 11), 10);
    ASSERT_EQ(::CMethods::get_adjusted_scaling_factor(-10, -11), -10);
    ASSERT_EQ(::CMethods::get_adjusted_scaling_factor(-11, -10), -10);
}

// Test the additive linear scaling procedure
TEST_F(TestCMethods, CheckLinearScalingAdditive) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "+";  // default

    std::vector<float> *result = new std::vector<float>(days);  // 10950 entries
    ::CMethods::Linear_Scaling(
        *result, *reference_temp, *control_temp, *scenario_temp, settings
    );

    const double mbe_before = std::abs(mbe(*target_temp, *scenario_temp));
    const double mbe_after = std::abs(mbe(*target_temp, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the multiplicative linear scaling procedure
TEST_F(TestCMethods, CheckLinearScalingMultiplicative) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "*";

    std::vector<float> *result = new std::vector<float>(days);
    ::CMethods::Linear_Scaling(
        *result, *reference_prec, *control_prec, *scenario_prec, settings
    );

    const double mbe_before = std::abs(mbe(*target_prec, *scenario_prec));
    const double mbe_after = std::abs(mbe(*target_prec, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the (additive) variance scaling procedure
TEST_F(TestCMethods, CheckVarianceScalingAdditive) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "+";  // default

    std::vector<float> *result = new std::vector<float>(days);
    ::CMethods::Variance_Scaling(
        *result, *reference_temp, *control_temp, *scenario_temp, settings
    );

    const double mbe_before = std::abs(mbe(*target_temp, *scenario_temp));
    const double mbe_after = std::abs(mbe(*target_temp, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the additive delta method procedure
TEST_F(TestCMethods, CheckDeltaMethodAdditive) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "+";  // default

    std::vector<float> *result = new std::vector<float>(days);  // 10950 entries
    ::CMethods::Delta_Method(
        *result, *reference_temp, *control_temp, *scenario_temp, settings
    );

    const double mbe_before = std::abs(mbe(*target_temp, *scenario_temp));
    const double mbe_after = std::abs(mbe(*target_temp, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the multiplicative delta method procedure
TEST_F(TestCMethods, CheckDeltaMethodMultiplicative) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "*";

    std::vector<float> *result = new std::vector<float>(days);
    ::CMethods::Delta_Method(
        *result, *reference_prec, *control_prec, *scenario_prec, settings
    );

    const double mbe_before = std::abs(mbe(*target_prec, *scenario_prec));
    const double mbe_after = std::abs(mbe(*target_prec, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the additive quantile mapping procedure
TEST_F(TestCMethods, CheckQuantileMappingAdditive) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "+";  // default

    std::vector<float> *result = new std::vector<float>(days);  // 10950 entries
    ::CMethods::Quantile_Mapping(
        *result, *reference_temp, *control_temp, *scenario_temp, settings
    );

    const double mbe_before = std::abs(mbe(*target_temp, *scenario_temp));
    const double mbe_after = std::abs(mbe(*target_temp, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the multiplicative quantile mapping procedure
TEST_F(TestCMethods, CheckQuantileMappingMultiplicative) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "*";

    std::vector<float> *result = new std::vector<float>(days);
    ::CMethods::Quantile_Mapping(
        *result, *reference_prec, *control_prec, *scenario_prec, settings
    );

    const double mbe_before = std::abs(mbe(*target_prec, *scenario_prec));
    const double mbe_after = std::abs(mbe(*target_prec, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the additive quantile delta mapping procedure
TEST_F(TestCMethods, CheckQuantileDeltaMappingAdditive) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "+";  // default

    std::vector<float> *result = new std::vector<float>(days);  // 10950 entries
    ::CMethods::Quantile_Delta_Mapping(
        *result, *reference_temp, *control_temp, *scenario_temp, settings
    );

    const double mbe_before = std::abs(mbe(*target_temp, *scenario_temp));
    const double mbe_after = std::abs(mbe(*target_temp, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}

// Test the multiplicative quantile delta mapping procedure
TEST_F(TestCMethods, CheckQuantileDeltaMappingMultiplicative) {
    AdjustmentSettings settings = AdjustmentSettings();
    settings.kind = "*";

    std::vector<float> *result = new std::vector<float>(days);
    ::CMethods::Quantile_Delta_Mapping(
        *result, *reference_prec, *control_prec, *scenario_prec, settings
    );

    const double mbe_before = std::abs(mbe(*target_prec, *scenario_prec));
    const double mbe_after = std::abs(mbe(*target_prec, *result));

    ASSERT_LE(mbe_after, mbe_before);
    delete result;
}
}  // namespace
}  // namespace CMethods
}  // namespace TestBiasAdjustCXX
