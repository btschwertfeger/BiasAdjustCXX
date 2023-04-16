# BiasadjustCXX Command-Line Tool for the Application of Fast and Efficient Bias Corrections in Climatic Research

<div align="center">

[![GitHub](https://badgen.net/badge/icon/github?icon=github&label)](https://github.com/btschwertfeger/BiasAdjustCXX)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-orange.svg)](https://www.gnu.org/licenses/gpl-3.0)
![C++](https://img.shields.io/badge/-C++-blue?logo=c%2B%2B)
![Build and Test](https://github.com/btschwertfeger/BiasAdjustCXX/actions/workflows/cicd.yml/badge.svg)
![Docker-pulls](https://img.shields.io/docker/pulls/btschwertfeger/biasadjustcxx.svg)

![release](https://shields.io/github/release-date/btschwertfeger/BiasAdjustCXX)
![release](https://shields.io/github/v/release/btschwertfeger/BiasAdjustCXX?display_name=tag)
![GCC](https://img.shields.io/badge/required-C%2B%2B11-green)
![CMake](https://img.shields.io/badge/required-CMake3.10-green)

[![DOI](https://zenodo.org/badge/495881923.svg)](https://zenodo.org/badge/latestdoi/495881923)
[![Publication DOI](https://img.shields.io/badge/Documentation-doi.org%2F10.1016%2Fj.softx.2023.101379-blue)](https://doi.org/10.1016/j.softx.2023.101379)

</div>

The BiasAdjustCXX command-line tool is the subject of a publication that provides an insight into the architecture, possible applications and new scientific questions. This publication referencing [BiasAdjustCXX v1.8.1](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.8.1) was published in the journal SoftwareX in March 2023 and is available at [https://doi.org/10.1016/j.softx.2023.101379](https://doi.org/10.1016/j.softx.2023.101379).

In addition - most of these methods available here have also been implemented in Python. This can be found in the [python-cmethods](https://github.com/btschwertfeger/Bias-Adjustment-Python) package.

---

## Table of Contents

1. [ About ](#about)
2. [ Available Methods ](#methods)
3. [ Compilation and Requirements ](#compilation)
4. [ Arguments and Parameters](#arguments)
5. [ Usage and Examples ](#examples)
6. [ Notes ](#notes)
7. [ References ](#references)

---

<a name="about"></a>

## 1. About

The BiasAdjustCXX command-line tool is the subject of a publication by Schwertfeger, Benjamin Thomas, Lohmann, Gerrit, and Lipskoch, Henrik (2023) _"Introduction of the BiasAdjustCXX command-line tool for the application of fast and efficient bias corrections in climatic research"_. It provides an insight into the architecture, possible applications and new scientific questions. This publication referencing [BiasAdjustCXX v1.8.1](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.8.1) was published in the journal SoftwareX in March 2023 and is available at [https://doi.org/10.1016/j.softx.2023.101379](https://doi.org/10.1016/j.softx.2023.101379).


These tool and data structures are developed with the aim of reducing discrepancies between modeled and observed climate data. Historical data is utilized to calibrate variables from current and future time series to achieve distributional properties that closely resemble the possible actual values.

<figure>
  <img
  src="images/biasCdiagram.png?raw=true"
  alt="Schematic representation of a bias adjustment procedure"
  style="background-color: white; border-radius: 7px">
  <figcaption>Figure 1: Schematic representation of a bias adjustment procedure</figcaption>
</figure>

For instance, modeled data typically indicate values that are colder than the actual values. To address this issue, an adjustment procedure is employed. The figure below illustrates the observed, modeled, and adjusted values, revealing that the delta adjusted time series ($T^{\*DM}{sim,p}$) are significantly more similar to the observed data ($T{obs,p}$) than the raw modeled data ($T_{sim,p}$).

<figure>
  <img
  src="images/dm-doy-plot.png?raw=true"
  alt="Temperature per day of year in modeled, observed and bias-adjusted climate data"
  style="background-color: white; border-radius: 7px">
  <figcaption>Figure 2: Temperature per day of year in observed, modeled, and bias-adjusted climate data</figcaption>
</figure>

Für fragen, anmerkungen, hilfestellungne, ideen oder kooperationen, kann jederzeit ein [BiasAdjustCXX/issues](https://github.com/btschwertfeger/BiasAdjustCXX/issues) auf gemacht, der discussion bereich genutzt [BiasAdjustCXX/discussions](https://github.com/btschwertfeger/BiasAdjustCXX/discussions), oder sich direkt an contact@b-schwertfeger.de gewandt werden

---

<a name="methods"></a>

## 2. Available Methods

### Distribution-based techniques:

- Quantile Mapping (additive and multiplicative)
- Quantile Delta Mapping (additive and multiplicative)

### Scaling-based techniques:

- Delta (Change) Method (additive and multiplicative)
- Linear Scaling (additive and multiplicative)
- Variance Scaling (additive)

#### General Notes:

- Except for the variance scaling, all methods can be applied on stochastic and non-stochastic
  climate variables. Variance scaling can only be applied on non-stochastic climate variables.

  - Stochastic climate variables are those that are subject to random fluctuations
    and are not predictable. They have no predictable trend or pattern. Examples of
    stochastic climate variables include precipitation, air temperature, and humidity.

  - Non-stochastic climate variables, on the other hand, have clear trend and pattern histories
    and can be readily predicted. They are often referred to as climate elements and include
    variables such as water temperature and air pressure.

- The Delta Method requires that the time series of the control period have the same length as the time series to be adjusted.

<a name="notes-scaling"> </a>

#### Notes regarding the scaling-based techniques:

- All data sets must exclude the 29th February and every year must have 365 entries. This is not required when using the `--no-group` flag which can be used to apply the scaling techniques in such a way that the scaling factors are based on the whole time series at once. This enables the possibility to apply the BiasAdjustCXX tool to data sets with custom time scales for example to adjust monthly separated time series individually to match the techniques described by Teutschbein ([2012](https://doi.org/10.1016/j.jhydrol.2012.05.052)) and Beyer ([2020](https://doi.org/10.5194/cp-16-1493-2020)). On the other hand the long-term 31-day interval procedures are customized variations and prevent disproportionately high differences in the long-term mean values at the monthly transitions. Thats why the long-term 31-day interval variant is the preferred method and is enabled by default for all scaling-based techniques.

---

<a name="compilation"></a>

## 3. Compilation and Requirements

#### 📍 If you are familiar with Docker, you can also use the Docker image as shown in Section [3.3 Alternative: Docker](#docker).

Otherwise - you can build BiasAdjustCXX from source as described below.

### 3.1 Compilation and installation:

```bash
git clone https://github.com/btschwertfeger/BiasAdjustCXX.git
cd BiasAdjustCXX

make build
make install
```

Uninstall using:

```bash
make uninstall
```

### 3.2 Compilation requirements/dependencies:

- NetCDF-4 C++ library ([How to install NetCDF-4 C++](https://github.com/Unidata/netcdf-cxx4))
- CMake v3.10+ ([How to install CMake](https://cmake.org/install/))
- [optional] Climate Data Operators ([How to install cdo](https://www.isimip.org/protocol/preparing-simulation-files/cdo-help/))

Optional for working example notebook `/examples/examples.ipynb` (only for hardliner):

```bash
conda create --name clingenv
conda activate clingenv
conda install xeus-cling notebook -c conda-forge/label/gcc7
```

(There is also an `/examples/environment.yml` file that creates a working environment for this notebook.)

<a name="docker"></a>

### 3.3 Alternative: Docker 🐳

The execution of BiasAdjustCXX is also possiblie within a Docker container. This is the preferred option when the installation of [NetCDF-4 C++](https://github.com/Unidata/netcdf-cxx4), [CMake](https://cmake.org) or BiasAdjustCXX on the local system is not desired. It also makes easier to access this tool since Docker container can run on nearly every operating system.

```bash
# remove the comments before execution ...
docker run -it -v $(PWD):/work btschwertfeger/biasadjustcxx:latest BiasAdjustCXX \
  --ref input_data/observations.nc  \ # observations/reference time series of the control period
  --contr input_data/control.nc     \ # simulated time series of the control period
  --scen input_data/scenario.nc     \ # time series to adjust
  -o linear_scaling.nc              \ # output file
  -m linear_scaling                 \ # adjustment method
  -k "+"                            \ # kind of adjustment ('+' == 'add' and '*' == 'mult')
  -v tas                            \ # variable to adjust
  -p 2                                # number of threads
```

See the Dockerhub registry to access the dev, pinned and older versions: [https://hub.docker.com/repository/docker/btschwertfeger/biasadjustcxx/general](https://hub.docker.com/repository/docker/btschwertfeger/biasadjustcxx/general)

### 3.4 Data requirements:

- The variable of interest must have the same name in all data sets.
- The dimensions must be named "time", "lat" and "lon" (i.e. time, latitudes and longitudes) in exactly this order in case the data sets have more than one dimension.
- Executed scaling-based techniques without the `--no-group` flag require that the data sets exclude the 29th February and every year has exactly 365 entries (see [Notes regarding the scaling-based techniques](#notes-scaling)).

---

<a name="arguments"></a>

## 4. Arguments and Parameters

| <div style="width:170px">Argument</div> | Description                                                                                                                                                                                                                                                                                                                 |
| --------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `--ref`, <br> `--reference`             | path to observational/reference data set(control period)                                                                                                                                                                                                                                                                    |
| `--contr`, <br> `--control`             | path to modeled data set (control period)                                                                                                                                                                                                                                                                                   |
| `--scen`, <br> `--scenario`             | path to data set that is to be adjusted (scenario period)                                                                                                                                                                                                                                                                   |
| `-v`, <br> `--variable`                 | variable to adjust                                                                                                                                                                                                                                                                                                          |
| `-k`, <br> `--kind`                     | kind of adjustment; one of: `+` or `add` and `*` or `mult`                                                                                                                                                                                                                                                                  |
| `-m`, <br> `--method`                   | adjustment method name; one of: `linear_scaling`, `variance_scaling`, `delta_method`, `quantile_mapping` and `quantile_delta_mapping`                                                                                                                                                                                       |
| `-q`, <br> `--n_quantiles`              | [optional] number of quantiles to respect (only required for distribution-based methods)                                                                                                                                                                                                                                    |
| `--1dim`                                | [optional] required if the data sets have no spatial dimensions (i.e. only one time dimension)                                                                                                                                                                                                                              |
| `--no-group`                            | [optional] Disables the adjustment based on 31-day long-term moving windows for the scaling-based methods. Scaling will be performed on the whole data set at once, so it is recommended to separate the input files for example by month and apply this program to every long-term month. (only for scaling-based methods) |
| `--max-scaling-factor`                  | [optional] Define the maximum scaling factor to avoid unrealistic results when adjusting ratio based variables for example in regions where heavy rainfall is not included in the modeled data and thus creating disproportional high scaling factors. (only for multiplicative methods except QM; default: 10)             |
| `-p`, <br> `--processes`                | [optional] How many threads to use (default: 1)                                                                                                                                                                                                                                                                             |
| `-h`, `--help`                          | [optional] display usage example, arguments, hints, and exits the program                                                                                                                                                                                                                                                   |

---

<a name="examples"></a>

## 5. Usage and Examples

The script `/examples/example_all_methods.run.sh` serves as an example on how to adjust the example data using all implemented methods.
`/examples/Hands-On-BiasAdjustCXX.ipynb` shows how to clone, build, compile, and run the `BiasAdjustCXX` command-line tool. Also plots validating the results are presented here.

All methods to bias-adjust climate data can be found in `/src/CMethods.cxx`. These can be imported into a Jupyter Notebook (with C++ kernel) to test scripts and develop custom algorithms (see `/examples/examples.ipynb`).

Examples:

a.) Additive Linear Scaling based on means of long-term 31-day intervals applied on a non-stochastic variable like temperatures ("tas"):

```bash
BiasAdjustCXX                        \
    --ref input_data/observations.nc \ # observations/reference time series of the control period
    --contr input_data/control.nc    \ # simulated time series of the control period
    --scen input_data/scenario.nc    \ # time series to adjust
    -o linear_scaling_result.nc      \ # output file
    -m linear_scaling                \ # adjustment method
    -k "+"                           \ # kind of adjustment ("+" == "add" and "*" == "mult")
    -v tas                           \ # variable to adjust
    -p 4                             \ # number of threads
```

Note/alternative: The regular linear scaling procedure as described by Teutschbein ([2012](https://doi.org/10.1016/j.jhydrol.2012.05.052)) needs to be applied on monthly separated data sets. To do so, you have to separate the input data sets into individual long-term months and apply the tool on each of these long-term months using the `--no-group` flag. This is shown in `/examples/example_all_methods.run.sh`.

b.) Multiplicative Delta Method based on means of long-term 31-day intervals applied on a stochastic variable like precipitation ("pr"):

```bash
BiasAdjustCXX                        \
    --ref input_data/observations.nc \
    --contr input_data/control.nc    \
    --scen input_data/scenario.nc    \
    -o delta_method_result.nc        \
    -m delta_method                  \
    -k "*"                           \
    -v pr                            \
    -p 4                             \ # use 4 threds (only if the input data is 3-dimensinoal)
    --max-scaling-factor 3             # set custom max-scaling factor to avoid unrealistic results (default: 10)
```

Note: The multiplicative variant is only preferred when adjusting non-stochastic variables like precipitaiton.

c.) Additive Quantile Delta Mapping applied on a non-stochastic variable like temperatures ("tas"):

```bash
BiasAdjustCXX                           \
    --ref input_data/observations.nc    \
    --contr input_data/control.nc       \
    --scen input_data/scenario.nc       \
    -o quantile_delta_mapping_result.nc \
    -m quantile_delta_mapping           \
    -k "+"                              \
    -v tas                              \
    -p 4                                \
    -q 250                                # quantiles to respect
```

d.) Help

```bash
BiasAdjustCXX --help
```

---

<a name="notes"></a>


## 📍 6. Notes

- For adjusting data using the linear scaling, variance scaling or delta method and the `--no-group` flag: You have to separate the input files by month and then apply the correction for each month individually. e.g. For 30 years of data to correct, you need to prepare the three input data sets so that they first contain all time series for all Januaries and then apply the adjustment for this data set. After that you have to do the same for the rest of the months (see `/examples/example_all_methods.run.sh`).

- Formulas and references can be found below and at the implementation of the corresponding functions.

- Speed/Performance tests and comparison to other tools can be found here: https://github.com/btschwertfeger/BiasAdjustCXX-Performance-Test

---

<a name="references"></a>

## 🔬 7. References

- Schwertfeger, Benjamin Thomas, Lohmann, Gerrit, and Lipskoch, Henrik (2023): _"Introduction of the BiasAdjustCXX command-line tool for the application of fast and efficient bias corrections in climatic research"_. (https://doi.org/10.1016/j.softx.2023.101379)
- Schwertfeger, Benjamin Thomas (2022) _"The influence of bias corrections on variability, distribution, and correlation of temperatures in comparison to observed and modeled climate data in Europe"_ (https://epic.awi.de/id/eprint/56689/)
- Linear Scaling and Variance Scaling based on: Teutschbein, Claudia and Seibert, Jan (2012) _"Bias correction of regional climate model simulations for hydrological climate-change impact studies: Review and evaluation of different methods"_ (https://doi.org/10.1016/j.jhydrol.2012.05.052)
- Delta Method based on: Beyer, R. and Krapp, M. and Manica, A. (2020): _"An empirical evaluation of bias correction methods for palaeoclimate simulations"_ (https://doi.org/10.5194/cp-16-1493-2020)
- Quantile Mapping based on: Alex J. Cannon and Stephen R. Sobie and Trevor Q. Murdock _"Bias Correction of GCM Precipitation by Quantile Mapping: How Well Do Methods Preserve Changes in Quantiles and Extremes?"_ (https://doi.org/10.1175/JCLI-D-14-00754.1)
- Quantile Delta Mapping based on: Tong, Y., Gao, X., Han, Z. et al. _"Bias correction of temperature and precipitation over China for RCM simulations using the QM and QDM methods"_. Clim Dyn 57, 1425–1443 (2021). (https://doi.org/10.1007/s00382-020-05447-4)
- Schulzweida, U.: _"CDO User Guide"_, https://doi.org/10.5281/zenodo.7112925, 2022.
- This project took advantage of netCDF software developed by UCAR/Unidata (http://doi.org/10.5065/D6H70CW6).

---
