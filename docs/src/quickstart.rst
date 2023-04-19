.. _section-installation:

Compilation and Installation
=============================


.. _build-from-source:

Build from source
----------------------------
Since this tool is written in C++ it must be compiled and installed, before it can be used.
The following libraries and tools must be installed to successfully compile and install
the BiasAdjustCXX command-line tool.

- NetCDF-4 C++ library (`How to install NetCDF-4 C++`_)
- CMake v3.10+ (`How to install CMake`_)
- [optional] Climate Data Operators (`How to install cdo`_)

Please have a look at the following code blocks that demonstrate how to download, build and install
the BiasAdjustCXX tool from source:


.. code-block:: bash
    :caption: Compilation and installation

    git clone https://github.com/btschwertfeger/BiasAdjustCXX.git
    cd BiasAdjustCXX

    make build
    make install

The tool can be uninstalled using the following command within the project directory:

.. code-block:: bash
    :caption: Uninstallation

    make uninstall


.. _section-docker:

Docker 🐳
--------------

The execution of BiasAdjustCXX is also possiblie within a Docker container.
This is the preferred option when the installation of `NetCDF-4 C++`_, `CMake`_ or `BiasAdjustCXX`_
on the local system is not desired. It also makes easier to access this tool since Docker
container can run on nearly every operating system.

.. code-block:: bash
    :caption: Run the BiasAdjustCXX tool using the provided Docker image

    # remove the comments before execution ...
    docker run -it -v $(PWD):/work btschwertfeger/biasadjustcxx:latest BiasAdjustCXX \
        --ref input_data/observations.nc  \ # observations/reference time series of the control period
        --contr input_data/control.nc     \ # simulated time series of the control period
        --scen input_data/scenario.nc     \ # time series to adjust
        -o linear_scaling.nc              \ # output file
        -m linear_scaling                 \ # adjustment method
        -k "+"                            \ # kind of adjustment ('+' == 'add' and '*' == 'mult')
        -v tas                            \ # variable to adjust
        -p 4                                # number of threads


See the Dockerhub registry to access the dev, pinned and older versions: `Dockerhub`_
