# -*- coding:utf-8 -*-
# Copyright (C) 2023 Benjamin Thomas Schwertfeger
# Github: https://github.com/btschwertfeger
#

FROM alpine:3.17

RUN apk add --update linux-headers libc-dev g++ build-base git cmake libaec-dev netcdf-dev hdf5-dev curl-dev

# download and instal NetCDFCXX
RUN git clone https://github.com/Unidata/netcdf-cxx4.git \
    && cd netcdf-cxx4 \
    && cmake -S . -B build \
    && cmake --build build \
    && cd build \
    && ctest \
    && make install \
    && rm -rf ../../netcdf-cxx4

# download, build and install BiasAdjustCXX
RUN git clone https://github.com/btschwertfeger/BiasAdjustCXX.git \
    && cd BiasAdjustCXX \
    && cmake -S . -B build \
    && cmake --build build \
    && cd build \
    && ctest \
    && cp BiasAdjustCXX /usr/local/bin \
    && rm -rf ../../BiasAdjustCXX
