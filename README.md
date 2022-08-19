# Bias-Adjustment-Cpp

[![Generic badge](https://img.shields.io/badge/license-MIT-green.svg)](https://shields.io/)
[![GitHub](https://badgen.net/badge/icon/github?icon=github&label)](https://github.com/btschwertfeger/Bias-Adjustment-Cpp)

Collection of different scale- and distribution-based bias adjustments for climatic research. Many of these methods have also been implemented in Python. This can be found here: https://github.com/btschwertfeger/Bias-Adjustment-Python.

____
## About
This repository started in 2022 as part of a Bachelor Thesis with the topic: ["The influence of bias corrections on variability, distribution, and correlation of temperatures in comparison to observed and modeled climate data in Europe"](https://b-schwertfeger.de/downloads/thesis_.pdf)

These programs and data structures are designed to help minimize discrepancies between modeled and observed climate data. Data from past periods are used to adjust variables from current and future time series so that their distributional properties approximate possible actual values.

<figure>
  <img
  src="images/biasCdiagram.png?raw=true"
  alt="Schematic representation of a bias adjustment procedure"
  style="background-color: white; border-radius: 7px">
  <figcaption>Figure 1: Schematic representation of a bias adjustment procedure</figcaption>
</figure>

In this way, for example, modeled data, which on average represent values that are too cold, can be adjusted by applying an adjustment procedure. The following figure shows the observed, the modeled and the adjusted values. It is directly visible that the delta adjusted time series ($T^{*DM}_{sim,p}$) are much more similar to the observed data ($T_{obs,p}$) than the raw modeled data ($T_{sim,p}$).

<figure>
  <img
  src="images/dm-doy-plot.png?raw=true"
  alt="Temperature per day of year in modeled, observed and bias-adjusted climate data"
  style="background-color: white; border-radius: 7px">
  <figcaption>Figure 2: Temperature per day of year in modeled, observed and bias-adjusted climate data</figcaption>
</figure>

____
## Available methods:
- Linear Scaling (additive and multiplicative)
- Variance Scaling (additive)
- Delta (Change) Method (additive and multiplicative)
- Quantile Mapping (additive and multiplicative)
- Quantile Delta Mapping (additive and multuplicative)
____
## Compilation:
```bash
./build.sh
```
or build form source:
```bash
mkdir build && cd build
cmake .. && cmake --build .
```

____
## Usage example
```bash
./Main                               \
    --ref input_data/observations.nc \
    --contr input_data/control.nc    \
    --scen input_data/scenario.nc    \
    -v tas                           \
    -m quantile_delta_mapping        \
    -q 100
```
The scipt `example_all_methods.run.sh` serves as an aexample on how to adjust the example data using all implemented methods. 

All methods to bias-adjust climate data can be found in `/src/CMethods.cxx`. These can be imported into a Jupyter Notebook (with c++ kernel) to test scripts and develop custom algorithms (see `/examples.ipynb`).
____
## Help
```bash
./Main -h
```
____
## Notes
- For adjusting data using the linear scaling, variance scaling or delta method, you have to separate the files by month and then apply the correction for each month individually. e.g. For 30 years of data to correct, you need to create a data set that contains all data for all januarys and then apply the adjustment for this data set. After that you have to do the same for the rest of the months.

____
## Requirements:
- Climate Data Operators ([How to install cdo](https://www.isimip.org/protocol/preparing-simulation-files/cdo-help/))
- Installed NetCDF4 C++ Library ([How to install NetCDF4 for C++](https://docs.geoserver.org/stable/en/user/extensions/netcdf-out/nc4.html))

### Optional for working examples in notebook (`examples.ipynb`)
```bash
conda create --name clingenv
conda activate clingenv
conda install xeus-cling notebook -c conda-forge/label/gcc7
```
_____
## Contributions
... are welcome (: