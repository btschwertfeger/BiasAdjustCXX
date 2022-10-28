// -*- lsst-c++ -*-

/**
 * @file Utils.hxx
 * @brief Utility functions
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 *
 *  * Copyright (C) 2022  Benjamin Thomas Schwertfeger
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
 *
 */

#ifndef __UTILS__
#define __UTILS__
#include <CMethods.hxx>
#include <iostream>

/*
 * ----- ----- ----- C O N T E N T ----- ----- -----
 */

namespace utils {
/** Class to create log files */
class Log {
   public:
    Log();
    ~Log();

    static void debug(std::string message);
    static void info(std::string message);
    static void warning(std::string message);
    static void error(std::string message);

   private:
};
bool isInStrV(std::vector<std::string> v, std::string string);
void progress_bar(float part, float all);
void show_license();
void show_usage(std::string name);

}  // namespace utils

#endif

/*
 * ----- ----- ----- E O F ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */
