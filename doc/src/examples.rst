Usage Examples
==============


- All referenced scripts in the following section can be found in the
  `/examples`_ directory within repository of the `BiasAdjustCXX`_ command-line tool.

- The script `/examples/example_all_methods.run.sh`_ serves as an example on how to adjust
  the example data (also provided in the repository) using all implemented methods.

- `/examples/Hands-On-BiasAdjustCXX.ipynb`_ shows how to clone, build, compile, and
  run the `BiasAdjustCXX`_ command-line tool. Also plots validating the results are presented here.

- All methods to bias-adjust climate data can be found in ``/src/CMethods.cxx``. These can be
  imported into a Jupyter Notebook (with C++ kernel) to test scripts and develop
  custom algorithms (see `/examples/examples.ipynb`_).

- All examples can also be run using the provided Docker image mentioned in
  section :ref:`section-installation`.

- *... More exmples can be found in section:* :ref:`Available Methods`


Additive Linear Scaling
------------------------

Additive Linear Scaling based on means of long-term 31-day intervals applied on a
non-stochastic variable like temperatures ("tas"):

.. code-block:: bash
    :caption: Additive Linear Scaling

    BiasAdjustCXX                        \
        --ref input_data/observations.nc \ # observations/reference time series of the control period
        --contr input_data/control.nc    \ # simulated time series of the control period
        --scen input_data/scenario.nc    \ # time series to adjust
        -o linear_scaling_result.nc      \ # output file
        -m linear_scaling                \ # adjustment method
        -k "+"                           \ # kind of adjustment ("+" == "add" and "*" == "mult")
        -v tas                           \ # variable to adjust
        -p 4                               # use 4 threads (only if the input data is 3-dimensional)


Note/alternative: The regular linear scaling procedure as described by
`Teutschbein et al. (2012)`_ needs to be applied on monthly separated data sets.
To do so, you have to separate the input data sets into individual long-term
months and apply the tool on each of these long-term months using the
``--no-group`` flag. This is shown in `/examples/example_all_methods.run.sh`_.

Multiplicative Delta Method
------------------------------------------------
Multiplicative Delta Method based on means of long-term 31-day intervals applied on a
stochastic variable like precipitation ("pr"):

.. code-block:: bash
    :caption: Multipliative Delta Method

    BiasAdjustCXX                        \
        --ref input_data/observations.nc \
        --contr input_data/control.nc    \
        --scen input_data/scenario.nc    \
        -o delta_method_result.nc        \
        -m delta_method                  \
        -k "*"                           \
        -v pr                            \
        --max-scaling-factor 3             # set custom max-scaling factor to avoid unrealistic results (default: 10)


Note: The multiplicative variant is only preferred when adjusting non-stochastic
variables like precipitation, air pressure or wind speed.

Additive Quantile Delta Mapping
------------------------------------
Additive Quantile Delta Mapping applied on a non-stochastic variable like temperatures ("tas"):

.. code-block:: bash
    :caption: Additive Quantile Delta Mapping

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


Help
------
.. code-block:: bash
    :caption: Show the help

    BiasAdjustCXX --help
