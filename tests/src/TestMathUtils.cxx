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
    }

    void TearDown() override {
        std::cout.rdbuf(sbuf);
        sbuf = nullptr;
    }

    std::stringstream buffer{};
    std::streambuf *sbuf;
};

// Test if available methods have not changed
TEST(TestMathUtils, CheckAvailableMethods) {
    EXPECT_EQ(::MathUtils::available_methods, std::vector<std::string>({"rmse", "mbe", "ioa", "corr", "sd", "var", "mean"}));
}

// Test if available methods that require one data set have not changed
TEST(TestMathUtils, CheckMethodsThatRequireOneDataset) {
    EXPECT_EQ(::MathUtils::requires_1_ds, std::vector<std::string>({"sd", "var", "mean"}));
}

// Test if available methods that require tow data sets have not changed
TEST(TestMathUtils, CheckMethodsThatRequireTwoDatasets) {
    EXPECT_EQ(::MathUtils::requires_2_ds, std::vector<std::string>({"rmse", "mbe", "ioa", "corr"}));
}

}  // namespace
}  // namespace MathUtils
}  // namespace TestBiasAdjustCXX
