Validation of the bias-corrected time-series
=======================================================

⚠️ This software is not actively maintained!

``ComputeIndicatorCXX`` enables generating a matrix that contains for example the :math:`RMSE`
for every time series between two data sets. This requires that the input data sets are 3-dimensional.

The purpose of this tool is to demonstrate that the bias-corrected time series are
a significantly better approximation to the reference data than the raw modeled data of the scenario period.


Compilation
-------------
Run the following command from the root of the BiasAdjustCXX project to compile and install the tool:

```bash
make build-val
make install-val
```

To uninstall:

```bash
make uninstall-val
```

Available methods
--------------------

- Mean Bias Error (mbe)
- Root Mean Square Error (rmse)
- Pearson Correlation Coefficient (corr)
- Standard Deviation (sd)
- Variance (var)
- Mean (mean)
- Index of Agreement (ioa)

Usage example
---------------

```bash
ComputeIndicatorCXX                     \
    -i output/linear_scaling_result.nc  \ # data set to evaluate
    -i input_data/observations.nc       \ # reference data
    -o output/mean_bias_error_ls_obs.nc \ # output file path
    -v tas                              \ # variable to adjust
    -m mbe                              \ # method to use
```
