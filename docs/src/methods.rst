.. _section-available-methods:

Available Methods
=================

The implemented bias correction techniques can be devided into scaling-based and distribution-based
methods. While scaling-based methods try to minimize the deviations in the mean values by adding
or multiplying values so called "scaling factors", distribution-based techniques apply distributional
transformations often called "CDF transformations".

The mathematical basis of the scaling-based bias correction techniques Linear Scaling (LS),
Variance Scaling (VS) and the Delta Method (DM) are described in `Teutschbein et al. (2012)`_ and `Beyer et al. (2020)`_.
During the development of the BiasAdjustCXX command-line tool a weak point of these techniques
was found - *the unrealistic mean values in the monthly transitions*. Since the scaling-based
techniques described in the articles are scaling/adjusting the time-series in the long-term monthly
mean values the scaling factors of the individual months are completly differnt, so that for example
all Januaries are scaled by 1.5, but Februaries by 1.2 which can lead to high and unrealistic deviations
in the long-term monthly mean.

Since these weak point was detected, a new approach was developed: *the scaling based on long-term 31-day
moving windows*. This technque ensures that values are scaled based on the long-term values of the
surrounding entries. This technique is the default in BiasAdjustCXX. To disable this behaviour and
apply the scaling on the whole time-seroes at once the ``--no-group``-flag can be used.

The month-dependent scaling described in the mentioned articles is not implemented, but can be
applied as demonstrated in the provided example script `/examples/example_all_methods.run.sh`_
within the reposiory.

The distribution-based bias correction techniques are implemented based on the mathematical
formlas described by `Cannon et al. (2015)`_ and `Tong et al. (2021)`_.

Except for the Variance Scaling all methods can be applied on both, stochastic and non-stochastic
variables. The Variance Scaling can only be applied on stochastic climate variables.

  - Stochastic climate variables are those that are subject to random fluctuations
    and are not predictable. They have no predictable trend or pattern. Examples of
    stochastic climate variables include precipitation, air temperature, and humidity.

  - Non-stochastic climate variables, on the other hand, have clear trend and pattern histories
    and can be readily predicted. They are often referred to as climate elements and include
    variables such as water temperature and air pressure.

Of course all examples shown at the methods can be executed using the provided Docker image. See
section :ref:`section-installation` for more details.

Linear Scaling
------------------------
The Linear Scaling bias correction technique can be applied on stochastic and
non-stochastic climate variables to minimize deviations in the mean values
between predicted and observed time-series of past and future time periods.

Since the multiplicative scaling can result in very high scaling factors,
a maximum scaling factor of 10 is set. This can be changed by passing
another value to the optional ``--max-scaling-factor`` argument.

The Linear Scaling bias correction technique implemented here is based on the
method described in the equations of `Teutschbein et al. (2012)`_
*"Bias correction of regional climate model simulations for hydrological climate-change
impact studies: Review and evaluation of different methods"* but using long-term 31-day moving windows instead
of long-term monthly means (because of the weak point mentioned in section :ref:`section-available-methods`).

In the following the equations for both additive and multiplicative Linear Scaling are shown:

**Additive**:

In linear scaling, the mean of the long-term 31-day moving window (:math:`\mu_m`) of the modeled
data :math:`X_{sim,h}` is subtracted from mean of the the long-term 31-day moving window
of the reference data :math:`X_{obs,h}` at time step :math:`i`. This difference in the mean is than added
to the value of time step :math:`i` in the time-series that is to be adjusted (:math:`X_{sim,p}`).

.. math::

    X^{*LS}_{sim,p}(i) = X_{sim,p}(i) + \mu_{m}(X_{obs,h}(i)) - \mu_{m}(X_{sim,h}(i))


**Multiplicative**:

    The multiplicative linear scaling differs from the additive variant in such way, that the changes are not computed
    in absolute but in relative values.

    .. math::

        X^{*LS}_{sim,h}(i) = X_{sim,h}(i) \cdot \left[\frac{\mu_{m}(X_{obs,h}(i))}{\mu_{m}(X_{sim,h}(i))}\right]


**Example**:

The following example shows how to apply the additive linear scaling technique on a
3-dimensional data set containing the variable "tas" (i.e., temperatures).

.. code-block:: bash
    :linenos:
    :caption: Example: Linear Scaling

    BiasAdjustCXX                         \
        --ref input_data/observations.nc  \ # observations/reference time series of the control period
        --contr input_data/control.nc     \ # simulated time series of the control period
        --scen input_data/scenario.nc     \ # time series to adjust
        --output linear_scaling_result.nc \ # output file
        --method linear_scaling           \ # adjustment method
        --kind "+"                        \ # kind of adjustment ("+" == "add" and "*" == "mult")
        --variable tas                    \ # variable to adjust
        --processes 4                       # use 4 threads (only if the input data is 3-dimensional)


Variance Scaling
------------------------
The Variance Scaling bias correction technique can be applied only on non-stochastic
climate variables to minimize deviations in the mean and variance
between predicted and observed time-series of past and future time periods.

The Variance Scaling bias correction technique implemented here is based on the
method described by `Teutschbein et al. (2012)`_ *"Bias correction of regional climate model
simulations for hydrological climate-change impact studies: Review and evaluation of different methods"*
but using long-term 31-day moving windows instead of long-term monthly means
(because of the weak point mentioned in section :ref:`section-available-methods`).
In the following the equations of the variance scaling approach are shown:

**(1)** First, the modeled data of the control and scenario period must be bias-corrected using
the additive linear scaling technique. This adjusts the deviation in the mean.

.. math::

    X^{*LS}_{sim,h}(i) = X_{sim,h}(i) + \mu_{m}(X_{obs,h}(i)) - \mu_{m}(X_{sim,h}(i))

    X^{*LS}_{sim,p}(i) = X_{sim,p}(i) + \mu_{m}(X_{obs,h}(i)) - \mu_{m}(X_{sim,h}(i))


**(2)** In the second step, the time-series are shifted to a zero mean. This enables the adjustment
of the standard deviation in the following step.

.. math::

    X^{VS(1)}_{sim,h}(i) = X^{*LS}_{sim,h}(i) - \mu_{m}(X^{*LS}_{sim,h}(i))

    X^{VS(1)}_{sim,p}(i) = X^{*LS}_{sim,p}(i) - \mu_{m}(X^{*LS}_{sim,p}(i))


**(3)** Now the standard deviation (so variance too) can be scaled.

.. math::

    X^{VS(2)}_{sim,p}(i) = X^{VS(1)}_{sim,p}(i) \cdot \left[\frac{\sigma_{m}(X_{obs,h}(i))}{\sigma_{m}(X^{VS(1)}_{sim,h}(i))}\right]


**(4)** Finally the prevously removed mean is shifted back

.. math::

    X^{*VS}_{sim,p}(i) = X^{VS(2)}_{sim,p}(i) + \mu_{m}(X^{*LS}_{sim,p}(i))



**Example**:

The following example shows how to apply the (additive) variance scaling technique on a
3-dimensional data set containing the variable "tas" (i.e., temperatures).

.. code-block:: bash
    :linenos:
    :caption: Example: Variance Scaling

    BiasAdjustCXX                           \
        --ref input_data/observations.nc    \ # observations/reference time series of the control period
        --contr input_data/control.nc       \ # simulated time series of the control period
        --scen input_data/scenario.nc       \ # time series to adjust
        --output variance_scaling_result.nc \ # output file
        --method variance_scaling           \ # adjustment method
        --kind "+"                          \ # kind of adjustment (only additive is valid for VS)
        --variable tas                        # variable to adjust


Delta Method
------------------------
The Delta Method bias correction technique can be applied on stochastic and
non-stochastic climate variables to minimize deviations in the mean values
between predicted and observed time-series of past and future time periods.

Since the multiplicative scaling can result in very high scaling factors,
a maximum scaling factor of 10 is set. This can be changed by passing
another value to the optional ``--max-scaling-factor`` argument.

The Delta Method bias correction technique implemented here is based on the
method described in the equations of `Beyer et al. (2020)`_ *"An empirical evaluation of bias
correction methods for palaeoclimate simulations"* but using long-term 31-day moving windows
instead of long-term monthly means (because of the weak point mentioned in section :ref:`section-available-methods`).
In the following the equations for both additive and multiplicative Delta Method are shown:

**Additive**:

    The Delta Method looks like the Linear Scaling method but the important difference is, that the Delta method
    uses the change between the modeled data instead of the difference between the modeled and reference data of the control
    period. This means that the long-term monthly mean (:math:`\mu_m`) of the modeled data of the control period :math:`T_{sim,h}`
    is subtracted from the long-term monthly mean of the modeled data from the scenario period :math:`T_{sim,p}` at time step :math:`i`.
    This change in month-dependent long-term mean is than added to the long-term monthly mean for time step :math:`i`,
    in the time-series that represents the reference data of the control period (:math:`T_{obs,h}`).

    .. math::

        X^{*DM}_{sim,p}(i) = X_{obs,h}(i) + \mu_{m}(X_{sim,p}(i)) - \mu_{m}(X_{sim,h}(i))


**Multiplicative**:

    The multiplicative variant behaves like the additive, but with the difference that the change is computed using the relative change
    instead of the absolute change.

    .. math::

        X^{*DM}_{sim,p}(i) = X_{obs,h}(i) \cdot \left[\frac{ \mu_{m}(X_{sim,p}(i)) }{ \mu_{m}(X_{sim,h}(i))}\right]


**Example**:

The following example shows how to apply the multiplicative delta method technique on a
3-dimensional data set containing the variable "pr" (i.e., precipitaiton).

.. code-block:: bash
    :linenos:
    :caption: Example: Delta Method

    BiasAdjustCXX                           \
        --ref input_data/observations.nc    \ # observations/reference time series of the control period
        --contr input_data/control.nc       \ # simulated time series of the control period
        --scen input_data/scenario.nc       \ # time series to adjust
        --output delta_method_result.nc     \ # output file
        --method delta_method               \ # adjustment method
        --kind "*"                          \ # kind of adjustment
        --variable pr                         # variable to adjust




Quantile Mapping
------------------------
The Quantile Mapping bias correction technique can be used to minimize distributional
biases between modeled and observed time-series climate data. Its interval-independant
behaviour ensures that the whole time series is taken into account to redistribute
its values, based on the distributions of the modeled and observed/reference data of the
control period.

The Quantile Mapping technique implemented here is based on the equations of
`Cannon et al. (2015)`_ *"Bias Correction of GCM Precipitation by Quantile Mapping:
How Well Do Methods Preserve Changes in Quantiles and Extremes?"*.

A weak point of the regular Quantile Mapping is, that the values are bounded to the
value range of the modeled data of the control period.

In the following the equations of `Cannon et al. (2015)`_ are shown and explained:

**Additive**:

    .. math::

        X^{*QM}_{sim,p}(i) = F^{-1}_{obs,h} \left\{F_{sim,h}\left[X_{sim,p}(i)\right]\right\}


    The additive quantile mapping procedure consists of inserting the value to be
    adjusted (:math:`X_{sim,p}(i)`) into the cumulative distribution function
    of the modeled data of the control period (:math:`F_{sim,h}`). This determines,
    in which quantile the value to be adjusted can be found in the modeled data of the control period
    The following images show this by using :math:`T` for temperatures.

    .. figure:: ../_static/images/qm-cdf-plot-1.png
        :width: 600
        :align: center
        :alt: Determination of the quantile value

        Fig 1: Inserting :math:`X_{sim,p}(i)` into :math:`F_{sim,h}` to determine the quantile value

    This value, which of course lies between 0 and 1, is subsequently inserted
    into the inverse cumulative distribution function of the reference data of the control period to
    determine the bias-corrected value at time step :math:`i`.

    .. figure:: ../_static/images/qm-cdf-plot-2.png
        :width: 600
        :align: center
        :alt: Determination of the QM bias-corrected value

        Fig 1: Inserting the quantile value into :math:`F^{-1}_{obs,h}` to determine the bias-corrected value for time step :math:`i`

**Multiplicative**:

    .. math::

        X^{*QM}_{sim,p}(i) = F^{-1}_{obs,h}\Biggl\{F_{sim,h}\left[\frac{\mu{X_{sim,h}} \cdot \mu{X_{sim,p}(i)}}{\mu{X_{sim,p}(i)}}\right]\Biggr\}\frac{\mu{X_{sim,p}(i)}}{\mu{X_{sim,h}}}

**Example**:

The following example shows how to apply the multiplicative quantile mapping technique on a
3-dimensional data set containing the variable "pr" (i.e., precipitaiton).

.. code-block:: bash
    :linenos:
    :caption: Example: Quantile Mapping

    BiasAdjustCXX                           \
        --ref input_data/observations.nc    \ # observations/reference time series of the control period
        --contr input_data/control.nc       \ # simulated time series of the control period
        --scen input_data/scenario.nc       \ # time series to adjust
        --output quantile_mapping_result.nc \ # output file
        --method quantile_mapping           \ # adjustment method
        --kind "*"                          \ # kind of adjustment
        --variable pr                         # variable to adjust



Quantile Delta Mapping
------------------------

The Quantile Delta Mapping bias correction technique can be used to minimize distributional
biases between modeled and observed time-series climate data. Its interval-independant
behaviour ensures that the whole time series is taken into account to redistribute
its values, based on the distributions of the modeled and observed/reference data of the
control period. In contrast to the regular Quantile Mapping the Quantile Delta Mapping
also takes the change between the modeled data of the control and scenario period into account.

The Quantile Delta Mapping technique implemented here is based on the equations by
`Tong et al. (2021)`_ *"Bias correction of temperature and precipitation
over China for RCM simulations using the QM and QDM methods"*. In the following the formulas of the
additive and multiplicative variant are shown.

**Additive**:

    **(1.1)** In the first step the quantile value of the time step :math:`i` to adjust is stored in
    :math:`\varepsilon(i)`.

    .. math::

        \varepsilon(i) = F_{sim,p}\left[X_{sim,p}(i)\right], \hspace{1em} \varepsilon(i)\in\{0,1\}


    **(1.2)** The bias corrected value at time step :math:`i` is now determined by inserting the
    quantile value into the inverse cummulative distribution function of the reference data of the control
    period. This results in a bias corrected value for time step :math:`i` but still without taking the
    change in modeled data into account.

    .. math::

        X^{QDM(1)}_{sim,p}(i) = F^{-1}_{obs,h}\left[\varepsilon(i)\right]


    **(1.3)** The :math:`\Delta(i)` represents the absolute change in quantiles between the modeled value
    in the control and scenario period.

    .. math::

            \Delta(i) & = F^{-1}_{sim,p}\left[\varepsilon(i)\right] - F^{-1}_{sim,h}\left[\varepsilon(i)\right] \\[1pt]
                    & = X_{sim,p}(i) - F^{-1}_{sim,h}\left\{F^{}_{sim,p}\left[X_{sim,p}(i)\right]\right\}


    **(1.4)** Finally the previously calculated change can be added to the bias-corrected value.

    .. math::

        X^{*QDM}_{sim,p}(i) = X^{QDM(1)}_{sim,p}(i) + \Delta(i)


**Multiplicative**:

    The first two steps of the multiplicative Quantile Delta Mapping bias correction technique are the
    same as for the additive variant.

    **(2.3)** The :math:`\Delta(i)` in the multiplicative Quantile Delta Mapping is calulated like the
    additive variant, but using the relative than the absolute change.

        .. math::

            \Delta(i) & = \frac{ F^{-1}_{sim,p}\left[\varepsilon(i)\right] }{ F^{-1}_{sim,h}\left[\varepsilon(i)\right] } \\[1pt]
                        & = \frac{ X_{sim,p}(i) }{ F^{-1}_{sim,h}\left\{F_{sim,p}\left[X_{sim,p}(i)\right]\right\} }


    **(2.4)** The relative change between the modeled data of the control and scenario period is than
    multiplicated with the bias-corrected value (see **1.2**).

        .. math::

            X^{*QDM}_{sim,p}(i) = X^{QDM(1)}_{sim,p}(i) \cdot \Delta(i)


**Example**:

The following example shows how to apply the additive quantile delta mapping technique on a
3-dimensional data set containing the variable "tas" (i.e., temperatures).

.. code-block:: bash
    :linenos:
    :caption: Example: Quantile Delta Mapping

    BiasAdjustCXX                                 \
        --ref input_data/observations.nc          \ # observations/reference time series of the control period
        --contr input_data/control.nc             \ # simulated time series of the control period
        --scen input_data/scenario.nc             \ # time series to adjust
        --output quantile_delta_mapping_result.nc \ # output file
        --method quantile_delta_mapping           \ # adjustment method
        --kind "+"                                \ # kind of adjustment
        --variable tas                              # variable to adjust
