# Bias-Adjustment-Cpp
Collection of different scale- and distribution-based bias adjustments for climatic research. This methods are part of the bachelor thesis of Benjamin T. Schwertfeger. During this thesis, many of these methods have also been implemented in Python. This can be found here: https://github.com/btschwertfeger/Bias-Adjustment-Python.

## Compile:
```bash
./build.sh
```
## Example
```bash
./DoBiasAdjustment                   \
    --ref input_data/observations.nc \
    --contr input_data/control.nc    \
    --scen input_data/scenario.nc    \
    -v tas                           \
    -m quantile_delta_mapping        \
    -q 100
```
The scipt `example_all_methods.run.sh` serves as an aexample on how to adjust the example data using all implemented methods. 

## Help:
```bash
./DoBiasAdjustment -h
```



## Requirements:
- Climate Data Operators ([How to install cdo](https://www.isimip.org/protocol/preparing-simulation-files/cdo-help/))
- Installed NetCDF4 C++ Library ([How to install NetCDF4 for C++](https://docs.geoserver.org/stable/en/user/extensions/netcdf-out/nc4.html))