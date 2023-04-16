// -*- lsst-c++ -*-
/**
 * @file TestUtils.cxx
 * @brief Implements the unit tests of the Utils class
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

#include <vector>

#include "Utils.hxx"
#include "gtest/gtest.h"

namespace TestBiasAdjustCXX {
namespace Utils {
namespace {

// The fixture for testing class Utills.
class TestUtils : public ::testing::Test {
   protected:
    // You can remove any or all of the following functions if their bodies would
    // be empty.

    TestUtils() : sbuf{nullptr} {
        // You can do set-up work for each test here.
    }

    ~TestUtils() override {
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

// Test if Log class can write debug statements
TEST_F(TestUtils, CheckLogCanDebug) {
    const utils::Log log = utils::Log();
    std::string expected{
        "\x1B[37mDEBUG: \x1B[0mTest\n"};
    log.debug("Test");
    std::string actual{buffer.str()};
    EXPECT_EQ(expected, actual);
}

// Test if Log class can write info statements
TEST_F(TestUtils, CheckLogCanInfo) {
    const utils::Log log = utils::Log();
    std::string expected{
        "\x1B[32mINFO: \x1B[0mTest\n"};
    log.info("Test");
    std::string actual{buffer.str()};
    EXPECT_EQ(expected, actual);
}

// Test if Log class can write warning statements
TEST_F(TestUtils, CheckLogCanWarning) {
    const utils::Log log = utils::Log();
    std::string expected{
        "\x1B[33mWARNING: \x1B[0mTest\n"};
    log.warning("Test");
    std::string actual{buffer.str()};
    EXPECT_EQ(expected, actual);
}

// Test if Log class can write error statements
TEST_F(TestUtils, CheckLogCanError) {
    const utils::Log log = utils::Log();
    std::string expected{
        "\x1B[1m\x1B[31mERROR: \x1B[0mTest\n"};
    log.error("Test");
    std::string actual{buffer.str()};
    EXPECT_EQ(expected, actual);
}

// Tests the progress bar
TEST_F(TestUtils, CheckProgressBar) {
    std::string expected{
        "50 / 100 [ ###################################                                   ] 50 %\r"};
    utils::progress_bar(50.0, 100.0);
    std::string actual{buffer.str()};
    EXPECT_EQ(expected, actual);
}

// Tests if a string is in a vector
TEST_F(TestUtils, CheckStringInVector) {
    const std::string string_to_find = "linear_scaling";
    const std::vector<std::string> v({"linear_scaling", "delta_method"});
    EXPECT_EQ(utils::isInStrV(v, string_to_find), true);
}

// Tests if a string is not in a vector
TEST_F(TestUtils, CheckStringNotInVector) {
    const std::string string_to_find = "linear_scaling";
    const std::vector<std::string> v({"quantile_mapping", "delta_method"});
    EXPECT_EQ(utils::isInStrV(v, string_to_find), false);
}

// Tests the copyright notice
TEST_F(TestUtils, CheckCopyrightNotice) {
    std::string expected{
        "BiasAdjustCXX Copyright (C) 2023 Benjamin Thomas Schwertfeger"
        "\nThis program comes with ABSOLUTELY NO WARRANTY."
        "\nThis is free software, and you are welcome to redistribute it"
        "\nunder certain conditions; type 'show -c' for details.\n\n"};
    utils::show_copyright_notice("BiasAdjustCXX");
    std::string actual{buffer.str()};
    EXPECT_EQ(expected, actual);
}

}  // namespace
}  // namespace Utils
}  // namespace TestBiasAdjustCXX
