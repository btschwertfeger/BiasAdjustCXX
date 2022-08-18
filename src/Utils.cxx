// -*- lsst-c++ -*-

/**
 * @file Utils.cxx
 * @brief
 * @author Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp
 *
 */

/*
 * ----- ----- ----- I N C L U D E ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */

#include "Utils.hxx"

#include <iostream>

#include "colors.h"

/*
 * ----- ----- ----- L O G G E R ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */
namespace utils {
utils::Log::Log() {}
utils::Log::~Log() {}

void utils::Log::debug(std::string message) { std::cout << WHITE << "DEBUG: " << RESET << message << std::endl; }

void utils::Log::info(std::string message) { std::cout << GREEN << "INFO: " << RESET << message << std::endl; }

void utils::Log::warning(std::string message) { std::cout << YELLOW << "WARNING: " << RESET << message << std::endl; }

void utils::Log::error(std::string message) { std::cout << BOLDRED << "ERROR: " << RESET << message << std::endl; }
}  // namespace utils
/*
 * ----- ----- ----- F U N C T I O N S ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */

namespace utils {
/** Progress bar to bar the progress
 *  inspired by: https://stackoverflow.com/a/14539953/13618168
 *
 * @param part fraction of the process that is done
 * @param all total number of processes
 */

void progress_bar(float part, float all) {
    const float progress = part / all;
    const unsigned barWidth = 70;

    std::cout << part << " / " << all << " [ ";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
            std::cout << "#";
        else if (i == pos)
            std::cout << "";
        else
            std::cout << " ";
    }
    std::cout << " ] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}
}  // namespace utils

/*
 * ----- ----- ----- E O F ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- */
