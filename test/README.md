# Validation/Testing of the results

`ComputeIndicator.cxx` enables creating a matrix that contains for example the $RMSE$ of every time series between two data sets.

## Compilation:

```bash
./build.sh
```

or build form source:

```bash
mkdir build && cd build
cmake .. && cmake --build .
```

## Available methods:

- Mean Bias Error (mbe)
- Root Mean Square Error (rmse)
- Pearson Correlation Coefficient (corr)
- Standard Deviation (sd)
- Variance (var)
- Mean (mean)
- Index of Agreement (ioa)

## Usage example

```bash
ComputeIndicatorCXX                     \
    -i output/linear_scaling_result.nc  \ # result data set to evaluate
    -i input/observations.nc            \ # reference data
    -o output/mean_bias_error_ls_obs.nc \ # output file path
    -v tas                              \ # variable to adjust
    -m mbe                              \ # method to use
```
