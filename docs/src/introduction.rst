.. This is the introduction

Introduction
=============

|GitHub badge| |License badge| |C++ badge| |CICD badge|
|Docker pulls badge| |GCC badge| |CMake badge|
|Release date badge| |Release tag badge| |DOI badge| |Publication bage|


About
-----

The command-line tool `BiasAdjustCXX`_ is the subject of a publication that provides an
insight into the architecture, possible applications and new scientific questions. This publication referencing
`BiasAdjustCXX v1.8.1`_ was published in the journal SoftwareX in March 2023 and is available
at `https://doi.org/10.1016/j.softx.2023.101379`_.

This tool and the provided data structures are designed
to help minimize discrepancies between modeled and observed climate data of different
time periods. Data from past periods are used to adjust variables
from current and future time series so that their distributional
properties approximate possible actual values.


.. figure:: ../_static/images/biasCdiagram.png
    :width: 600
    :align: center
    :alt: Schematic representation of a bias adjustment procedure

    Fig 1: Schematic representation of a bias adjustment procedure


In this way, for example, modeled data, which on average represent values
that are too cold, can be bias-corrected by applying an adjustment procedure.
The following figure shows the observed, the modeled, and the bias-corrected values.
It is directly visible that the delta adjusted time series
(:math:`T^{*DM}_{sim,p}`) are much more similar to the observed data (:math:`T_{obs,p}`)
than the raw modeled data (:math:`T_{sim,p}`).

.. figure:: ../_static/images/dm-doy-plot.png
    :width: 600
    :align: center
    :alt: Temperature per day of year in modeled, observed and bias-adjusted climate data

    Fig 2: Temperature per day of year in modeled, observed and bias-adjusted climate data

In addition - most of these methods available here have also been implemented in Python.
This can be found in the `python-cmethods`_ package.

If you have any inquiries, remarks, requests for assistance, ideas, or potential collaborations,
you can always create an issue on `BiasAdjustCXX/issues`_, utilize the discussion area on
`BiasAdjustCXX/discussions`_, or directly contact me at contact@b-schwertfeger.de.


Available bias correction methods
---------------------------------

The following bias correction techniques are available:
    Scaling-based techniques:
        * :ref:`Linear Scaling`
        * :ref:`Variance Scaling`
        * :ref:`Delta Method`

    Distribution-based techniques:
        * :ref:`Quantile Mapping`
        * :ref:`Quantile Delta Mapping`

All of these mathematical methods are intended to be applied on 1-dimensional time-series climate data.
This module also provides the possibility that enables
the application of the desired bias correction method on 3-dimensinoal data sets.

General Notes
~~~~~~~~~~~~~

- Except for the variance scaling, all methods can be applied on stochastic and non-stochastic
  climate variables. Variance scaling can only be applied on non-stochastic climate variables.

  - Stochastic climate variables are those that are subject to random fluctuations
    and are not predictable. They have no predictable trend or pattern. Examples of
    stochastic climate variables include precipitation, air temperature, and humidity.

  - Non-stochastic climate variables, on the other hand, have clear trend and pattern histories
    and can be readily predicted. They are often referred to as climate elements and include
    variables such as water temperature and air pressure.

- The Delta Method requires that the time series of the control period have the same length
  as the time series to be adjusted.

- Examples can be found in the `BiasAdjustCXX`_ repository and of course
  within this documentation.

- Speed/Performance tests and comparison to other tools can be found here: `tool comparison`_

- References can be found in the :ref:`References` section.


.. _section-notes-scaling:

Notes regarding the scaling-based techniques
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- All data sets must exclude the 29th February and every year must have 365 entries.
  This is not required when using the ``--no-group`` flag which can be used to apply
  the scaling techniques in such a way that the scaling factors are based on the whole
  time series at once. This enables the possibility to apply the BiasAdjustCXX tool to data
  sets with custom time scales for example to adjust monthly separated time series individually to
  match the techniques described by `Teutschbein et al. (2012)`_ and `Beyer et al. (2020)`_. On the other hand the
  long-term 31-day interval procedures are customized variations and prevent
  disproportionately high differences in the long-term mean values at the monthly transitions.
  Thats why the long-term 31-day interval variant is the preferred method and is enabled by
  default for all scaling-based techniques.
