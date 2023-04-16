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
    }

    void TearDown() override {
    }
};

TEST_F(TestCMethods, CheckGetLongTermDayOfYear) {}
TEST_F(TestCMethods, CheckGetAdjustedScalingFactor) {}
TEST_F(TestCMethods, CheckLinearScaling) {}
TEST_F(TestCMethods, CheckVarianceScaling) {}
TEST_F(TestCMethods, CheckDeltaMethod) {}
TEST_F(TestCMethods, CheckQuantileMapping) {}
TEST_F(TestCMethods, CheckQuantileDeltaMapping) {}
}  // namespace
}  // namespace CMethods
}  // namespace TestBiasAdjustCXX
