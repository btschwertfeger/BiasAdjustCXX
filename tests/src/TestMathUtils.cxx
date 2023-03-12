// -*- lsst-c++ -*-
/**
 * @file TestMathUtils.c++
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
namespace Utils {
namespace {

// The fixture for testing class MathUtills.
class TestMathUtils : public ::testing::Test {
   protected:
    // You can remove any or all of the following functions if their bodies would
    // be empty.

    TestMathUtils() : sbuf{nullptr} {
        // You can do set-up work for each test here.
    }

    ~TestMathUtils() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
        // Save cout's buffer...
        sbuf = std::cout.rdbuf();
        // Redirect cout to our stringstream buffer or any other ostream
        std::cout.rdbuf(buffer.rdbuf());
    }

    // Called after each unit test
    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
        // When done redirect cout to its old self
        std::cout.rdbuf(sbuf);
        sbuf = nullptr;
    }

    // The following objects can be reused in each unit test

    // This can be an ofstream as well or any other ostream
    std::stringstream buffer{};
    // Save cout's buffer here
    std::streambuf *sbuf;
};

// Test if available methods have not changed
TEST(TestMathUtils, CheckAvailableMethods) {
    EXPECT_EQ(MathUtils::available_methods, std::vector<std::string>({"rmse", "mbe", "ioa", "corr", "sd", "var", "mean"}));
}

// Test if available methods that require one data set have not changed
TEST(TestMathUtils, CheckMethodsThatRequireOneDataset) {
    EXPECT_EQ(MathUtils::requires_1_ds, std::vector<std::string>({"sd", "var", "mean"}));
}

// Test if available methods that require tow data sets have not changed
TEST(TestMathUtils, CheckMethodsThatRequireTwoDatasets) {
    EXPECT_EQ(MathUtils::requires_2_ds, std::vector<std::string>({"rmse", "mbe", "ioa", "corr"}));
}

}  // namespace
}  // namespace Utils
}  // namespace TestBiasAdjustCXX
