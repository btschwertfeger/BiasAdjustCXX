#!/bin/zsh

# @brief Controled execution of the Main.app program to bias adjust example data sets
# @author Benjamin Thomas Schwertfeger
# @email development@b-schwertfeger.de
# @link https://b-schwertfeger.de
# @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp

# ? ------- ------- ------- ------- ------- ------- ------- ------- -------
# * -------------------------------------------------------------------
# *                  ===== Description =====
# * -------------------------------------------------------------------
# ? USAGE: ./example.run.adjust.sh
# ? - This script separates data sets by month and then
# ?    applies all bias adjustment methods.
# ? > interim results are saved in some temporary folder
# ? > clean up after computation
# * -------------------------------------------------------------------

work_dir=$(pwd)
mkdir -p "${work_dir}/output"
output_dir="${work_dir}/output"
rm -rf tmp/

timespan="19810101_20101231"
observations="${work_dir}/input_data/observations.nc"
control="${work_dir}/input_data/control.nc"
scenario="${work_dir}/input_data/scenario.nc"

variable="tas"
kind="+"
n_quantiles=100

exec_file="${work_dir}/build/BiasAdjustCXX"

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
# *                    ===== Computation =====
# * -------------------------------------------------------------------

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
            -q $n_quantiles
    done

    # ? Merge corrected datasets
    cdo -f nc \
        -mergetime $tmp_path/results/*${method}.nc \
        "${output_dir}/${variable}_${method}_kind-${kind}_${quant}result_${timespan}.nc"
done

# ? now the distribution-based adjustments
for method in "${quant_methods[@]}"; do
    $exec_file                                  \
        --ref $observations                     \
        --contr $control                        \
        --scen $scenario                        \
        -v $variable                            \
        -m $method                              \
        -k $kind                                \
        -q $n_quantiles                         \
        -o "${output_dir}/${variable}_${method}_kind-${kind}_quants-${n_quantiles}_result_${timespan}.nc"
done

# ? Adjustment of a data set with only one dimension (time)
$exec_file                                \
    --ref input_data/1d_observations.nc   \
    --contr input_data/1d_control.nc      \
    --scen input_data/1d_scenario.nc      \
    -m "quantile_mapping"                 \
    -v $variable                          \
    -q $n_quantiles                       \
    -k $kind                              \
    --1dim                                \
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
