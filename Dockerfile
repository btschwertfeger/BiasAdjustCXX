# -*- coding:utf-8 -*-
# Copyright (C) 2023 Benjamin Thomas Schwertfeger
# Github: https://github.com/btschwertfeger
#

FROM ubuntu:22.04

RUN apt-get update \
    && apt-get -y install cmake git build-essential libnetcdf-dev \
    && apt-get autoremove

# download and instal NetCDFCXX
RUN git clone https://github.com/Unidata/netcdf-cxx4.git \
    && cd netcdf-cxx4 \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make \
    && ctest \
    && make install \
    && rm -rf ../../netcdf-cxx4

# download, build and install BiasAdjustCXX
RUN git clone https://github.com/btschwertfeger/BiasAdjustCXX.git \
    && cd BiasAdjustCXX \
    && mkdir build && cd build \
    && cmake .. && cmake --build . \
    && cp BiasAdjustCXX /usr/local/bin \
    && rm -rf ../../BiasAdjustCXX

