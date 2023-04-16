// -*- lsst-c++ -*-
/**
 * @file TestManager.cxx
 * @brief Implements the unit tests of the manager class
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

#include "Manager.hxx"
#include "gtest/gtest.h"

namespace TestBiasAdjustCXX {
namespace Manager {
namespace {

// The fixture for testing class Manager.
class TestManager : public ::testing::Test {
   protected:
    TestManager() {
    }

    ~TestManager() override {
    }

    void SetUp() override {
    }

    void TearDown() override {
    }
};

// Not tested so far since this would required acceptance tests
}  // namespace
}  // namespace Manager
}  // namespace TestBiasAdjustCXX
