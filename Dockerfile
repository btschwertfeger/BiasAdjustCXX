# -*- coding:utf-8 -*-
# Copyright (C) 2023 Benjamin Thomas Schwertfeger
# Github: https://github.com/btschwertfeger
#
FROM alpine:3.17 AS builder

# hadolint ignore=DL3018
RUN apk add --update --no-cache \
    linux-headers \
    libc-dev \
    libaec-dev \
    netcdf-dev \
    hdf5-dev \
    curl-dev \
    g++ \
    build-base \
    cmake \
    git \
    rm -rf /var/cache/apk/*

# Install netcdf-cxx4
WORKDIR /netcdf-cxx4
# hadolint ignore=DL3003
RUN git clone https://github.com/Unidata/netcdf-cxx4.git /netcdf-cxx4 \
    && cmake -S . -B build \
    && cmake --build build \
    && cd build \
    && ctest \
    && make install

# Install BiasAdjustCXX
COPY . /BiasAdjustCXX/
WORKDIR /BiasAdjustCXX
RUN rm -rf build \
    && make dev \
    && make test \
    && make clean \
    && make build \
    && make install

# ===== N E X T - S T A G E ======

FROM alpine:3.17

# hadolint ignore=DL3018
RUN apk add --update --no-cache \
    libc-dev \
    libaec-dev \
    netcdf-dev \
    hdf5-dev \
    rm -rf /var/cache/apk/*

COPY --from=builder /usr/local/ /usr/local/

WORKDIR /work
