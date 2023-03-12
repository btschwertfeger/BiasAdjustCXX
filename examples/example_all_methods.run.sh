#!/bin/zsh

# @author Benjamin Thomas Schwertfeger
# @email development@b-schwertfeger.de
# @link https://b-schwertfeger.de
# @github https://github.com/btschwertfeger/BiasAdjustCXX
#
#    * Copyright (C) 2023 Benjamin Thomas Schwertfeger
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <https://www.gnu.org/licenses/>.


# ? ------- ------- ------- ------- ------- ------- ------- ------- -------
# * -------------------------------------------------------------------
# *                  ===== Description =====
# * -------------------------------------------------------------------
#   This script executes different bias correction techniques
#   and different configurations of the BiasAdjustCXX tool
# ? USAGE: ./example.run.adjust.sh
# ? OR: sh example.run.adjust.sh
# * -------------------------------------------------------------------

work_dir=$(pwd) # you may have to change the path here
mkdir -p "${work_dir}/output"
output_dir="${work_dir}/output"
rm -rf tmp/

input_dir="../input_data"
timespan="19810101_20101231"
observations="${input_dir}/observations.nc"
control="${input_dir}/control.nc"
scenario="${input_dir}/scenario.nc"

variable="tas"
kind="+"
n_quantiles=100

exec_file="${work_dir}/../build/BiasAdjustCXX"

declare -a month_methods=("delta_method" "linear_scaling" "variance_scaling")
declare -a quant_methods=("quantile_mapping" "quantile_delta_mapping")

# * -------------------------------------------------------------------
# *                   ===== Setup =====
# * -------------------------------------------------------------------

# enable array index at 0 for zsh
{ setopt KSH_ARRAYS || : ; } 2> /dev/null

tmp_path=${work_dir}/tmp/$(uuidgen | tr '[:upper:]' '[:lower:]' | head -c 12)
tmp_ref="${tmp_path}/ref"
tmp_contr="${tmp_path}/contr"
tmp_scen="${tmp_path}/scen"
tmp_results="${tmp_path}/results"

mkdir -p $tmp_ref
mkdir -p $tmp_contr
mkdir -p $tmp_scen
mkdir -p $tmp_results

declare -a datasets=("${observations}" "${control}" "${scenario}")
declare -a ds_paths=("${tmp_ref}" "${tmp_contr}" "${tmp_scen}")

# * -------------------------------------------------------------------
# *            ===== Distribution-based Adjustent =====
# * -------------------------------------------------------------------

for method in "${quant_methods[@]}"; do
    $exec_file                                  \
        --ref $observations                     \
        --contr $control                        \
        --scen $scenario                        \
        -v $variable                            \
        -m $method                              \
        -k $kind                                \
        -q $n_quantiles                         \
        -p 4                                    \
        -o "${output_dir}/${variable}_${method}_kind-${kind}_quants-${n_quantiles}_result_${timespan}.nc"
done



# * -------------------------------------------------------------------
# *  ===== Default Scaling Adjustent (long-term 31-day intervals) =====
# * -------------------------------------------------------------------

# ? Additive linear scaling based on 31 day long-term mean interval instead of
# long-term monthly means to avoid high deviations between month transitions
# this only works if every dataset has 365 days per year (no January 29th)
# this is available for all scaling methods

$exec_file                             \
    --ref $observations                \
    --contr $control                   \
    --scen $scenario                   \
    -m "linear_scaling"                \
    -v $variable                       \
    -k "add"                           \
    -p 4                               \
    -o "${output_dir}/${variable}_linear_scaling_kind-add_scalingtype-31dayinterval_result_${timespan}.nc"


# * -------------------------------------------------------------------
# *   ===== Scaling Adjustment based on long-term monthly means =====
# * -------------------------------------------------------------------

# ? OR: Adjust using the regular formulas using long-term monthly means by using the --no-group flag
echo "Separating months ..."
# ? Separate months
for (( month=1; month<13; month++ ));do
    declare -i index=0
    for dataset in "${datasets[@]}"; do
        cdo -f nc -s \
            -selvar,$variable \
            -selmon,$month $dataset \
            "${ds_paths[index]}/${month}.nc" &
        ((++index))
    done
    wait
done

echo "Staring adjusting data ..."
# ? this should not be done in parallel because this could lead to excessive memory usage
for method in "${month_methods[@]}"; do
    # ? Apply scaling-based bias adjustment per month
    for (( month=1; month<13; month++ )); do
        $exec_file                                          \
            --ref "${tmp_ref}/${month}.nc"                  \
            --contr "${tmp_contr}/${month}.nc"              \
            --scen "${tmp_scen}/${month}.nc"                \
            -v $variable                                    \
            -m $method                                      \
            -o "${tmp_results}/${month}_${method}.nc"       \
            -p 4                                            \
            --no-group
    done

    # ? Merge corrected datasets
    cdo -f nc \
        -mergetime $tmp_path/results/*${method}.nc \
        "${output_dir}/${variable}_${method}_kind-${kind}_scalingtype-monthly_${quant}result_${timespan}.nc"
done


# * -------------------------------------------------------------------
# *    ===== Adjustment of a 1-dimensional data set =====
# * -------------------------------------------------------------------

# ? Adjustment of a data set with only one dimension (time)
$exec_file                                              \
    --ref "${input_dir}/1d_observations.nc"             \
    --contr "${input_dir}/1d_control.nc"                \
    --scen "${input_dir}/1d_scenario.nc"                \
    -m "quantile_mapping"                               \
    -v $variable                                        \
    -q $n_quantiles                                     \
    -k $kind                                            \
    --1dim                                              \
    -o "${output_dir}/${variable}_1d_quantile_mapping_kind-${kind}_quants-${n_quantiles}_result_${timespan}.nc"

# * -------------------------------------------------------------------
# *                    ===== clean-up =====
# * -------------------------------------------------------------------

rm -rf tmp/
echo "Finished adjusting data sets!"
exit 0

# * -------------------------------------------------------------------
# *                     ===== EOF =====
# * -------------------------------------------------------------------
# * ------- ------- ------- ------- ------- ------- ------- ------- -------
