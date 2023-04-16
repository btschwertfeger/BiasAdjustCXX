# Validation/Testing of the adjusted time series

> ⚠️ This part is not actively maintained!

`ComputeIndicator.cxx` enables creating a matrix that contains for example the $RMSE$ of every time series between two data sets.

## Compilation:

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
    -i output/linear_scaling_result.nc  \ # data set to evaluate
    -i input_data/observations.nc       \ # reference data
    -o output/mean_bias_error_ls_obs.nc \ # output file path
    -v tas                              \ # variable to adjust
    -m mbe                              \ # method to use
```

## Notes

- The application of this program to the adjusted example time series will produce a very high approximation to the reference data.
