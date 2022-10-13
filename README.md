# Bias-Adjustment-Cpp

[![Generic badge](https://img.shields.io/badge/license-MIT-green.svg)](https://shields.io/)
[![GitHub](https://badgen.net/badge/icon/github?icon=github&label)](https://github.com/btschwertfeger/Bias-Adjustment-Cpp)

Collection of different scale- and distribution-based bias adjustment techniques for climatic research. Many of these methods have also been implemented in Python. This can be found here: https://github.com/btschwertfeger/Bias-Adjustment-Python.

____
## About
This repository started in 2022 as part of a Bachelor Thesis with the topic: ["The influence of bias corrections on variability, distribution, and correlation of temperatures in comparison to observed and modeled climate data in Europe"](https://b-schwertfeger.de/downloads/thesis.pdf)

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
- Quantile Mapping
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
./Main.app                           \
    --ref input_data/observations.nc \ # observations/reference time series of the control period
    --contr input_data/control.nc    \ # simulated time series of the control period
    --scen input_data/scenario.nc    \ # time series to adjust
    -v tas                           \ # variable to adjust
    -m quantile_delta_mapping        \ # adjustment method
    -k "+"                           \ # kind of qdjustment ("+" == "add" and "*" == "mult")
    -q 100                             # quantiles to respect 
```
The scipt `example_all_methods.run.sh` serves as an example on how to adjust the example data using all implemented methods. 

All methods to bias-adjust climate data can be found in `/src/CMethods.cxx`. These can be imported into a Jupyter Notebook (with c++ kernel) to test scripts and develop custom algorithms (see `/examples.ipynb`).
____
## Help
```bash
./Main.app -h
```
____
## Notes
- For adjusting data using the linear scaling, variance scaling or delta method, you have to separate the files by month and then apply the correction for each month individually. e.g. For 30 years of data to correct, you need to create a data set that contains all data for all januarys and then apply the adjustment for this data set. After that you have to do the same for the rest of the months.
- Formulas and references can be found in the implementations of the corresponding functions.

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

____
## References
- Schwertfeger, Benjamin Thomas (2022) The influence of bias corrections on variability, distribution, and correlation of temperatures in comparison to observed and modeled climate data in Europe (https://epic.awi.de/id/eprint/56689/)
- Linear Scaling and Variance Scaling based on: Teutschbein, Claudia and Seibert, Jan (2012) Bias correction of regional climate model simulations for hydrological climate-change impact studies: Review and evaluation of different methods (https://doi.org/10.1016/j.jhydrol.2012.05.052)
- Delta Method based on: Beyer, R. and Krapp, M. and Manica, A.: An empirical evaluation of bias correction methods for palaeoclimate simulations (https://doi.org/10.5194/cp-16-1493-2020)
- Quantile Mapping based on: Alex J. Cannon and Stephen R. Sobie and Trevor Q. Murdock Bias Correction of GCM Precipitation by Quantile Mapping: How Well Do Methods Preserve Changes in Quantiles and Extremes? (https://doi.org/10.1175/JCLI-D-14-00754.1)
- Quantile Delta Mapping based on: Tong, Y., Gao, X., Han, Z. et al. Bias correction of temperature and precipitation over China for RCM simulations using the QM and QDM methods. Clim Dyn 57, 1425–1443 (2021). (https://doi.org/10.1007/s00382-020-05447-4)
