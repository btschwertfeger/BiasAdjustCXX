Arguments and Requirements
===========================

Arguments
-------------

The following table lists the available command-line arguments that can be
passed to the `BiasAdjustCXX`_ tool. Please also have a look at the requirements
section below.

.. csv-table:: Command-line arguments
   :file: ../_static/arguments.csv
   :delim: ;
   :widths: 30, 70
   :header-rows: 1


Requirements
-------------

- The variable of interest must have the same name in all data sets.
- The dimensions must be named "time", "lat" and "lon" (i.e., time, latitudes
  and longitudes) in exactly this order - in case the data sets have more than
  one dimension.
- Executed scaling-based techniques without the ``--no-group`` flag require that
  the data sets exclude the 29th February and every year has exactly 365 entries
  (see :ref:`section-notes-scaling`).
- For adjusting data using the linear scaling, variance scaling or delta method
  and the ``--no-group`` flag: You have to separate the input files by month and
  then apply the correction for each month individually e.g., for 30 years of
  data to correct, you need to prepare the three input data sets so that they
  first contain all time series for all Januaries and then apply the adjustment
  for this data set. After that you have to do the same for the rest of the
  months (see `/examples/example_all_methods.run.sh`_ in the repository).
