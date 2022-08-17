#!/bin/zsh

# @brief Implementation of Bias Adjustment methods
# @author Benjamin Thomas Schwertfeger
# @copyright Benjamin Thomas Schwertfeger
# @link https://b-schwertfeger.de
# @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp

# ? ------- ------- ------- ------- ------- ------- ------- ------- ------- 
# * -------------------------------------------------------------------
# *                           Description
# * -------------------------------------------------------------------
# ? USAGE: ./example.run.adjust.sh 
# ? - This script separates datasets by month and then 
# ?   applies all bias adjustment methods.
# ? - interim results are saved in some tmp folder 
# ? - clean up after computation will be done
# ? - no commands except for help can be passed
# * -------------------------------------------------------------------

work_dir=$(pwd)
mkdir -p "${work_dir}/output"
output_dir="${work_dir}/output"
rm -rf tmp/

timespan="19810101_20101231"
observations="${work_dir}/input_data/obs.nc"
control="${work_dir}/input_data/contr.nc"
scenario="${work_dir}/input_data/scen.nc"

variable="tas"
kind="+"
n_quantiles=100  

exec_file="${work_dir}/Main"

declare -a month_methods=("delta_method" "linear_scaling" "variance_scaling")
declare -a quant_methods=("quantile_mapping" "quantile_delta_mapping")

# * -------------------------------------------------------------------
# *                      Setup
# * -------------------------------------------------------------------

# enable array index at 0 for zsh
{ setopt KSH_ARRAYS || : ; } 2> /dev/null

tmp_path=${work_dir}/tmp/$(uuidgen | tr '[:upper:]' '[:lower:]' | head -c 12)
tmp_obs="${tmp_path}/obs"
tmp_contr="${tmp_path}/contr"
tmp_scen="${tmp_path}/scen"
tmp_results="${tmp_path}/results"

mkdir -p $tmp_obs
mkdir -p $tmp_contr
mkdir -p $tmp_scen
mkdir -p $tmp_results

declare -a datasets=("${observations}" "${control}" "${scenario}")
declare -a ds_paths=("${tmp_obs}" "${tmp_contr}" "${tmp_scen}")

# * -------------------------------------------------------------------
# *                         Computation
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
            --ref "${tmp_obs}/${month}.nc"                  \
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
        -o "${output_dir}/${variable}_${method}_kind-${kind}_quants-${n_quantiles}_result_${timespan}.nc"         \
        -q $n_quantiles
done


# * -------------------------------------------------------------------
# *                         Clean-Up
# * -------------------------------------------------------------------

rm -rf $tmp_path
echo "Finished adjusting datasets!"
exit 0

# * -------------------------------------------------------------------
# *                         E O F 
# * -------------------------------------------------------------------
# * ------- ------- ------- ------- ------- ------- ------- ------- ------- 
