// -*- lsst-c++ -*-

/**
 * @file CMethods.hxx
 * @brief declaration of the CMethod class
 * @author Benjamin Thomas Schwertfeger
 * @copyright Benjamin Thomas Schwertfeger
 * @link https://b-schwertfeger.de
 * @github https://github.com/btschwertfeger/BiasAdjustCXX
 *
 *  * Copyright (C) 2022  Benjamin Thomas Schwertfeger
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef __MANAGER__
#define __MANAGER__

#include "NcFileHandler.hxx"
#include "Utils.hxx"

typedef void (*CM_Func_ptr_scaling_A)(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    bool interval31_scaling);

typedef void (*CM_Func_ptr_scaling_B)(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    double max_scaling_factor,
    bool interval31_scaling);

typedef void (*CM_Func_ptr_distribution)(
    std::vector<float>& v_output,
    std::vector<float>& v_reference,
    std::vector<float>& v_control,
    std::vector<float>& v_scenario,
    std::string kind,
    unsigned n_quantiles);

class Manager {
   public:
    Manager(int argc, char** argv);
    ~Manager();

    void run_adjustment();
    std::string get_adjustment_kind();

   private:
    void show_usage();
    void parse_args();

    void adjust_1d(
        std::vector<float>& v_data_out,
        std::vector<float>& v_reference,
        std::vector<float>& v_control,
        std::vector<float>& v_scenario);
    void adjust_3d(std::vector<std::vector<std::vector<float>>>& v_data_out);

    int argc;
    char** argv;

    CM_Func_ptr_scaling_A scaling_func_A;
    CM_Func_ptr_scaling_B scaling_func_B;
    CM_Func_ptr_distribution distribution_func;

    NcFileHandler* ds_reference;
    NcFileHandler* ds_control;
    NcFileHandler* ds_scenario;

    std::string variable_name;
    std::string output_filepath;
    std::string adjustment_method_name;
    std::string adjustment_kind;

    unsigned n_quantiles;
    double max_scaling_factor;
    bool one_dim;
    bool interval31_scaling;

    unsigned n_jobs;
    utils::Log log;
};
#endif
